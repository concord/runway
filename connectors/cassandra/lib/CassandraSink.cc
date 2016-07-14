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
                             const uint32_t maxAsyncInserts)
    : thisMetadata_(buildMetadata(name, inputStreams)),
      cluster_(keyspace, table, contactPoints),
      maxAsyncInserts_(maxAsyncInserts) {
  asyncInserts_.reserve(maxAsyncInserts);
}

void CassandraSink::init(CtxPtr ctx) {
  folly::Future<folly::Optional<std::string>> f = cluster_.connectSession();
  try {
    if (const folly::Optional<std::string> m = f.get(std::chrono::seconds(5))) {
      LOG(FATAL) << "Error on Cassandra connect: " << *m;
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
  // We have reached point where we will not continue to push to cassandra
  if (asyncInserts_.size() == maxAsyncInserts_) {
    waitOnAllFutures();
  }

  // Parse request, inserting resultant future into queue
  const folly::dynamic request = folly::parseJson(r.value);
  if (request.isObject()) {
    const folly::dynamic values = request["values"];
    if (values.isObject()) {
      folly::Future<bool> ins =
          cluster_.insert([&values](const std::string &col_name) {
            folly::dynamic data = values[col_name];
            return data.c_str();
          });
      asyncInserts_.push_back(std::move(ins));
      return;
    }
  }
  LOG(ERROR) << "Input stream request doesn't adhere to expected protocol";
}

void CassandraSink::waitOnAllFutures() {
  // Wait on all futures to complete, then clear queue
  folly::collectAll(asyncInserts_)
      .then([this](const std::vector<folly::Try<bool>> &tries) {
        for (const auto &t : tries) {
          if (t.hasException()) {
            LOG(ERROR) << "Exception detected in future: "
                       << t.exception().what();
            failedCallbacks_ += 1;
          } else if (t.hasValue() && t.value() == false) {
            failedCallbacks_ += 1;
          }
        }
      });
  asyncInserts_.clear();
}

const bolt::Metadata CassandraSink::buildMetadata(const std::string &name,
                                   const std::string &inputStreams) const {
  using StreamGpg = bolt::Metadata::StreamGrouping;
  std::set<StreamGpg> istreams;
  std::vector<std::string> names;
  folly::split(",", inputStreams, names);
  // TODO: The names array has a len of 1 if inputStreams==""
  std::transform(names.begin(), names.end(),
                 std::inserter(istreams, istreams.begin()),
                 [](const std::string s) {
                   return StreamGpg(s, bolt::Grouping::ROUND_ROBIN);
                 });
  return bolt::Metadata(name, istreams);
}
}
