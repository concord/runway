#include <gflags/gflags.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <concord/glog_init.hpp>
#include <concord/Computation.hpp>
#include <concord/time_utils.hpp>
#include "HighLevelKafkaProducer.hpp"

DEFINE_string(kafka_brokers, "127.0.0.1:9092", "seed kafka brokers");
DEFINE_string(kafka_topics, "", "coma delimited list of topics to produce to");
DEFINE_string(istreams, "", "coma delimed list of streams to listen on");

namespace concord {
class KafkaSink final : public bolt::Computation {
public:
  using CtxPtr = bolt::Computation::CtxPtr;
  KafkaSink(const std::string &istreams, const std::string brokerList,
            const std::string kafkaTopics)
      : producer_(splitString(brokerList)), istreams_(splitString(istreams)),
        topics_(splitString(kafkaTopics)) {
    LOG_IF(FATAL, topics_.empty()) << "Empty topics list";
  }

  virtual void init(CtxPtr ctx) override {
    LOG(INFO) << "KafkaSink connector initialized";
  }

  virtual void destroy() override {
    LOG(INFO) << "KafkaSink shutdown hook called";
  }

  virtual void processRecord(CtxPtr ctx, bolt::FrameworkRecord &&r) override {
    for (const auto &topic : topics_) {
      producer_.produce(topic, r.key, r.value);
    }
  }

  virtual void processTimer(CtxPtr ctx, const std::string &key,
                            int64_t time) override {
    throw std::runtime_error("This method not expected to be called");
  }

  virtual bolt::Metadata metadata() override {
    bolt::Metadata m;
    m.name = "kafka_sink_" + boost::algorithm::join(istreams_, ".");
    for (const auto &topic : istreams_) {
      m.istreams.insert({topic, bolt::Grouping::ROUND_ROBIN});
    }
    return m;
  }

private:
  static std::vector<std::string> splitString(const std::string &str) {
    std::vector<std::string> res;
    boost::algorithm::split(res, str, boost::is_any_of(","));
    return res;
  }

  HighLevelKafkaProducer producer_;
  const std::vector<std::string> istreams_;
  const std::vector<std::string> topics_;
};
}

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  bolt::logging::glog_init(argv[0]);
  bolt::client::serveComputation(
      std::make_shared<concord::KafkaSink>(FLAGS_istreams, FLAGS_kafka_brokers,
                                           FLAGS_kafka_topics),
      argc, argv);
  return 0;
}
