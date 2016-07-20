#pragma once
#include <string>
#include <vector>
#include <folly/futures/Future.h>
#include <concord/Computation.hpp>
#include "CassandraInserter.hpp"
namespace concord {
class CassandraSink : public bolt::Computation {
public:
  using CtxPtr = bolt::Computation::CtxPtr;

  CassandraSink(const std::string &keyspace, const std::string &table,
                const std::string &contactPoints, const std::string &name,
                const std::string &inputStreams,
                const uint64_t maxAsyncInserts);
  virtual ~CassandraSink() {}

  virtual void init(CtxPtr ctx) override;
  virtual void destroy() override;
  virtual void processRecord(CtxPtr ctx, bolt::FrameworkRecord &&r) override;
  virtual void processTimer(CtxPtr ctx, const std::string &key,
                            int64_t time) override {}

  virtual bolt::Metadata metadata() { return thisMetadata_; }

private:
  void waitOnAllFutures();
  static std::set<bolt::Metadata::StreamGrouping>
  buildInputStreams(const std::string &keyspace, const std::string &table,
                    std::string inputStreams);

  uint64_t failedCallbacks_{0};
  const bolt::Metadata thisMetadata_;
  CassandraInserter cluster_;
  std::vector<folly::Future<folly::Unit>> asyncInserts_;
  const uint64_t maxAsyncInserts_;
};
}
