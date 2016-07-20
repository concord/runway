#include "CassandraSink.hpp"
#include <set>
#include <chrono>
#include <folly/futures/FutureException.h>
#include <folly/json.h>
#include <folly/String.h>
#include <folly/dynamic.h>
#include <folly/ExceptionWrapper.h>
namespace concord {
CassandraSink::CassandraSink(const std::string &keyspace,
                             const std::string &table,
                             const std::string &contactPoints,
                             const std::string &name,
                             const std::string &inputStreams,
                             const uint64_t maxAsyncInserts)
    : thisMetadata_(bolt::Metadata(
          name, buildInputStreams(keyspace, table, inputStreams))),
      cluster_(keyspace, table, contactPoints),
      maxAsyncInserts_(maxAsyncInserts) {
  asyncInserts_.reserve(maxAsyncInserts);
}

void CassandraSink::init(CtxPtr ctx) {
  folly::Future<folly::Optional<std::string>> f = cluster_.connectSession();
  try {
    if (const folly::Optional<std::string> m = f.get(std::chrono::seconds(5))) {
      LOG(FATAL) << "Failed to connect to Cassandra; reason: " << *m;
    }
  } catch (folly::TimedOut e) {
    LOG(FATAL) << "Failed to connect to Cassandra within 5s timeout";
  }
}

void CassandraSink::destroy() {
  LOG(INFO) << "Waiting on remaining asynchronous events...";
  waitOnAllFutures();
  cluster_.disconnectSession();
  LOG(INFO) << "There were " << failedCallbacks_
            << " failed callbacks detected";
}

void CassandraSink::processRecord(CtxPtr ctx, bolt::FrameworkRecord &&r) {
  // If true stop creating asynchronous requests, wait for all to finish
  if (asyncInserts_.size() == maxAsyncInserts_) {
    waitOnAllFutures();
  }

  try {
    // Parse request, inserting resultant future into queue
    const folly::dynamic values = folly::parseJson(r.value)["values"];
    folly::Future<folly::Unit> ins =
        cluster_.insert([&values](const std::string &col_name) {
          const folly::dynamic &item = values[col_name];
          return item.isString() ? folly::sformat("'{}'", item.asString())
                                 : item.asString();
        });
    asyncInserts_.push_back(std::move(ins));
  } catch (const std::exception &e) {
    LOG(ERROR) << "Error during JSON parsing: " << e.what();
  }
}

void CassandraSink::waitOnAllFutures() {
  // Wait on all futures to complete, then clear queue
  folly::collectAll(asyncInserts_)
      .then([this](const std::vector<folly::Try<folly::Unit>> &tries) {
        for (const auto &t : tries) {
          const auto ex =
              t.withException<std::runtime_error>([](const std::exception &e) {
                LOG(ERROR) << "Exception detected post-future cb: " << e.what();
              });
          failedCallbacks_ += ex ? 1 : 0;
        }
      });
  asyncInserts_.clear();
}

std::set<bolt::Metadata::StreamGrouping>
CassandraSink::buildInputStreams(const std::string &keyspace,
                                 const std::string &table,
                                 std::string inputStreams) {
  using StreamGpg = bolt::Metadata::StreamGrouping;
  if (inputStreams.empty()) {
    inputStreams = folly::sformat("{}.{}", keyspace, table);
  }
  std::set<StreamGpg> istreams;
  std::vector<std::string> names;
  folly::split(",", inputStreams, names);
  std::transform(names.begin(), names.end(),
                 std::inserter(istreams, istreams.begin()),
                 [](const std::string s) {
                   return StreamGpg(s, bolt::Grouping::ROUND_ROBIN);
                 });
  return istreams;
}
}
