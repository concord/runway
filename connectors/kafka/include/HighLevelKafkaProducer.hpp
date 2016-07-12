#pragma once
#include <map>
#include <librdkafka/rdkafkacpp.h>
#include <glog/logging.h>
#include <folly/String.h>
#include "HighLevelKafkaClient.hpp"

namespace bolt {

class HighLevelKafkaProducer : public bolt::HighLevelKafkaClient,
                               public RdKafka::DeliveryReportCb {
  public:
  // The librdkafka producer API is awkward
  // This is a thin wrapper around it, so one can actually use it
  HighLevelKafkaProducer(const std::vector<std::string> &brokers,
                         const std::map<std::string, std::string> &opts = {});
  ~HighLevelKafkaProducer();

  // RdKafka::DeliveryReportCb methods
  void dr_cb(RdKafka::Message &message) override;

  void registerTopic(const std::string &topic);

  void produce(const std::string &topic,
               const std::string &key,
               const std::string &value,
               int32_t partition = RdKafka::Topic::PARTITION_UA);

  uint64_t bytesSent() const { return bytesSent_; }
  uint64_t msgsSent() const { return msgsSent_; }
  uint64_t bytesKafkaSendError() const { return bytesKafkaSendError_; }
  uint64_t msgsKafkaSendError() const { return msgsKafkaSendError_; }
  uint64_t bytesKafkaReceived() const { return bytesKafkaReceived_; }
  uint64_t msgsKafkaReceived() const { return msgsKafkaReceived_; }

  private:
  struct HighLevelKafkaProducerTopic {
    private:
    std::unique_ptr<RdKafka::Topic> createTopic() {
      CHECK(!topicName.empty()) << "Empty topic name";
      std::string err;
      auto topic = std::unique_ptr<RdKafka::Topic>(
        RdKafka::Topic::create(producer, topicName, topicConfig.get(), err));
      LOG_IF(FATAL, !err.empty()) << "Failed to create topic: " << topicName
                                  << ". Error: " << err;
      return topic;
    }

    public:
    HighLevelKafkaProducerTopic(RdKafka::Producer *producer,
                                const std::string &topicName)
      : producer(CHECK_NOTNULL(producer))
      , topicName(topicName)
      , topic(createTopic()) {}

    RdKafka::Producer *producer;
    const std::string topicName;
    const std::unique_ptr<RdKafka::Conf> topicConfig{
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC)};
    const std::unique_ptr<RdKafka::Topic> topic{nullptr};
  };


  std::map<std::string, std::string>
  defaultOptions(const std::vector<std::string> &brokers);

  std::unique_ptr<RdKafka::Producer> buildProducer();

  uint64_t bytesSent_{0};
  uint64_t msgsSent_{0};
  uint64_t bytesKafkaSendError_{0};
  uint64_t msgsKafkaSendError_{0};
  uint64_t bytesKafkaReceived_{0};
  uint64_t msgsKafkaReceived_{0};
  const std::unique_ptr<RdKafka::Producer> producer_{nullptr};
  std::unordered_map<std::string, std::unique_ptr<HighLevelKafkaProducerTopic>>
    topicConfigs_{};
};
}
