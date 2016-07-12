#include <concord/Computation.hpp>
#include <chrono>
#include <folly/futures/Future.h>
#include <folly/futures/FutureException.h>
#include "CassandraCluster.hpp"
namespace concord {
class CassandraComputation : public bolt::Computation {
public:
  using CtxPtr = bolt::Computation::CtxPtr;

  CassandraComputation() : cluster_("localhost") {
    folly::Future<bool> f = cluster_.connectSession("keyspace", "mytbl");
    try {
      f.within(std::chrono::seconds(2));
      LOG(INFO) << "Success!";
    } catch (folly::TimedOut) {
      LOG(FATAL) << "Timed out after attempting to connect to db within 2s";
    }
  }

  virtual void init(CtxPtr ctx) override {}
  virtual void destroy() override {}
  virtual void processRecord(CtxPtr ctx, bolt::FrameworkRecord &&r) override {}
  virtual void processTimer(CtxPtr ctx, const std::string &key,
                            int64_t time) override {}
  virtual bolt::Metadata metadata() override {
    bolt::Metadata m;
    m.name = "cassandara-connector";
    m.ostreams.insert("LMAO");
    return m;
  }

private:
  CassandraCluster cluster_;
};
}

int main(int argc, char *argv[]) {
  concord::CassandraComputation c;
  return 0;
}
