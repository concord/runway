#pragma once
#include <map>
#include <librdkafka/rdkafkacpp.h>
#include <folly/String.h>
#include <glog/logging.h>
#include "HighLevelKafkaClient.hpp"

namespace concord {
struct KafkaConsumerTopicMetadata {
  KafkaConsumerTopicMetadata(const std::string &topicName,
                             const bool fromBegining = false)
    : topicName(topicName)
    , startOffset(fromBegining ? RdKafka::Topic::OFFSET_BEGINNING :
                                 RdKafka::Topic::OFFSET_STORED) {}
  const std::string topicName;
  const int64_t startOffset;
  bool offset_set = false;
};

struct KafkaConsumerTopicMetrics {
  uint64_t bytesReceived{0};
  int64_t currentOffset{0};
  uint64_t msgsReceived{0};
  void updateMetrics(const RdKafka::Message *msg) {
    currentOffset = msg->offset();
    bytesReceived += msg->len();
    ++msgsReceived;
  }
};


// TODO(agallego) - add ability to read a properties file
//
// THE ONLY IMPORTANT callback is RdKafka::RebalanceCb. This callback is
// necessary  for setting:
//
// *  (librdkafka's CONFIGURATION.md) group.id, session.timeout.ms,
// *      partition.assignment.strategy, etc.
//
class HighLevelKafkaConsumer : public concord::HighLevelKafkaClient,
                               public RdKafka::RebalanceCb {
  public:
  HighLevelKafkaConsumer(const std::vector<std::string> &brokers,
                         const std::vector<KafkaConsumerTopicMetadata> &topics,
                         const std::map<std::string, std::string> &opts = {});
  ~HighLevelKafkaConsumer();

  // returns a temporary every time
  std::string name() const { return consumer_->name(); }

  // blocking call
  template <typename Func> void consume(Func fn) {
    bool run = true;
    while(systemRun_ && run) {
      auto m = std::unique_ptr<RdKafka::Message>(consumer_->consume(10));
      switch(m->err()) {
      case RdKafka::ERR__TIMED_OUT:
        LOG(ERROR) << "Kafka timedout: " << m->errstr();
        run = false;
        break;
      case RdKafka::ERR_NO_ERROR:
        resetRetryCount();
        updateMetrics(m.get());
        run = fn(std::move(m));
        break;
      case RdKafka::ERR__PARTITION_EOF:
        LOG(WARNING) << "Reached end of partition: (" << m->topic_name() << ":"
                     << m->partition() << ") " << m->errstr();
        break;
      case RdKafka::ERR__UNKNOWN_TOPIC:
      case RdKafka::ERR__UNKNOWN_PARTITION:
        LOG(FATAL) << "Consume failed. Unknown parition|topic: " << m->errstr();
        break;

      default:
        LOG(ERROR) << "Consume failed. Uknown reason: " << m->errstr();
        run = false;
        break;
      }
    }
  }

  // RdKafka::RebalanceCb
  virtual void
  rebalance_cb(RdKafka::KafkaConsumer *consumer,
               RdKafka::ErrorCode err,
               std::vector<RdKafka::TopicPartition *> &partitions) override;

  private:
  static std::map<std::string, std::string>
  defaultOptions(const std::vector<std::string> &brokers,
                 const std::vector<std::string> &topicNames);

  static std::vector<std::string>
  getTopicNames(const std::vector<KafkaConsumerTopicMetadata> &topicMetadata);

  std::unique_ptr<RdKafka::KafkaConsumer> buildKafkaConsumer();

  void updateMetrics(RdKafka::Message *msg) {
    topicMetrics_[msg->topic_name()][msg->partition()].updateMetrics(msg);
  }

  // Used to exit the consume loop in the case that an instance of this
  // class is being destructed
  bool systemRun_{true};

  const std::unique_ptr<RdKafka::Conf> defaultTopicConf_{
    RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC)};
  std::vector<KafkaConsumerTopicMetadata> topicMetadata_{};
  const std::unique_ptr<RdKafka::KafkaConsumer> consumer_{nullptr};
  std::unordered_map<std::string,
                     std::unordered_map<int32_t, KafkaConsumerTopicMetrics>>
    topicMetrics_{};
};
}
