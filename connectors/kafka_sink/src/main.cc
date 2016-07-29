#include <gflags/gflags.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <concord/glog_init.hpp>
#include <concord/Computation.hpp>
#include <concord/time_utils.hpp>
#include "HighLevelKafkaProducer.hpp"

DEFINE_string(kafka_brokers, "127.0.0.1:9092", "seed kafka brokers");
DEFINE_string(kafka_topics, "", "coma delimited list of topics");

namespace concord {
class KafkaSink final : public bolt::Computation {
public:
  using CtxPtr = bolt::Computation::CtxPtr;
  KafkaSink(const std::string brokerList, const std::string kafkaTopics)
      : producer_(splitString(brokerList)),
        istreams_(splitString(kafkaTopics)) {
    LOG_IF(FATAL, istreams_.empty()) << "Empty topics list";
  }

  virtual void init(CtxPtr ctx) override {
    LOG(INFO) << "KafkaSink connector initialized";
  }
  virtual void destroy() override {}
  virtual void processRecord(CtxPtr ctx, bolt::FrameworkRecord &&r) override {
    for (const auto& topic : istreams_) {
      producer_.produce(topic, r.key, r.value);
    }
  }
  virtual void processTimer(CtxPtr ctx, const std::string &key,
                            int64_t time) override {}
  virtual bolt::Metadata metadata() override {
    bolt::Metadata m;
    m.name = "kafka_sink_" + boost::algorithm::join(istreams_, ".");
    for (const auto& topic : istreams_) {
      m.istreams.insert({topic, bolt::Grouping::GROUP_BY});
    }
    return m;
  }

private:
  static std::vector<std::string> splitString(const std::string &str) {
    std::vector< std::string > res;
    boost::algorithm::split(res, str, boost::is_any_of(","));
    return res;
  }

  HighLevelKafkaProducer producer_;
  const std::vector<std::string> istreams_;  
};
}

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  bolt::logging::glog_init(argv[0]);
  bolt::client::serveComputation(std::make_shared<concord::KafkaSink>(
                                     FLAGS_kafka_brokers, FLAGS_kafka_topics),
                                 argc, argv);
  return 0;
}
