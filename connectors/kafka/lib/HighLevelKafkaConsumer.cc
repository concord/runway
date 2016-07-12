#include "HighLevelKafkaConsumer.hpp"
#include "Random.hpp"
#include <gflags/gflags.h>

DEFINE_string(kafka_consumer_debug,
              "",
              "all,generic,broker,topic,"
              "metadata,producer,queue,msg,"
              "protocol,cgrp,security,fetch");

namespace bolt {

std::ostream &operator<<(std::ostream &o, const KafkaConsumerTopicMetrics &m) {
  o << "Offset: " << m.currentOffset << ", bytesReceived: " << m.bytesReceived
    << ", msgsReceived: " << m.msgsReceived;
  return o;
}

HighLevelKafkaConsumer::HighLevelKafkaConsumer(
  const std::vector<std::string> &brokers,
  const std::vector<KafkaConsumerTopicMetadata> &topics,
  const std::map<std::string, std::string> &opts)
  : HighLevelKafkaClient(defaultOptions(brokers, getTopicNames(topics)), opts)
  , topicMetadata_(topics)
  , consumer_(buildKafkaConsumer()) {
  const auto topicNames = getTopicNames(topicMetadata_);
  LOG_IF(FATAL,
         consumer_->subscribe(topicNames) != RdKafka::ErrorCode::ERR_NO_ERROR)
    << "Could not subscribe consumers to: " << folly::join(", ", topicNames);
  LOG(INFO) << "Configuration: " << folly::join(" ", *clusterConfig_->dump());
}

HighLevelKafkaConsumer::~HighLevelKafkaConsumer() {
  systemRun_ = false;
  for(const auto &s : topicMetrics_) {
    for(const auto &m : s.second) {
      LOG(INFO) << "Stop topic: " << s.first << ", partition: " << m.first
                << ", metrics: " << m.second;
    }
  }
  consumer_->commitSync();
  consumer_->unassign();
  consumer_->close();
}

std::unique_ptr<RdKafka::KafkaConsumer>
HighLevelKafkaConsumer::buildKafkaConsumer() {
  std::string err;
  LOG_IF(FATAL,
         clusterConfig_->set("rebalance_cb", (RdKafka::RebalanceCb *)this, err)
           != RdKafka::Conf::CONF_OK)
    << err;
  LOG_IF(FATAL,
         clusterConfig_->set("default_topic_conf", defaultTopicConf_.get(), err)
           != RdKafka::Conf::CONF_OK)
    << err;
  // TOPIC!
  // The partitions in the TopicPartitions list in RebalanceCb will always
  // be INVALID(-1001) since no offset fetching from the broker has
  // taken place yet. Passing a partition with offset=INVALID to assign()
  // tells it to retrieve the stored offset from the broker, or if
  // that is not available revert to auto.offset.reset setting, similar w/
  // the  other constants.
  //
  // auto.offset.reset Action to take when there is no initial offset in
  // offset store or the desired offset is out of range:
  // 'smallest','earliest'
  // - automatically reset the offset to the smallest offset,
  // 'largest','latest' - automatically reset the offset to the largest
  // offset, 'error' - trigger an error which is retrieved by consuming
  // messages and checking 'message->err'.
  //
  LOG_IF(FATAL, defaultTopicConf_->set("auto.offset.reset", "latest", err)
                  != RdKafka::Conf::CONF_OK)
    << err;

  auto consumer = std::unique_ptr<RdKafka::KafkaConsumer>(
    RdKafka::KafkaConsumer::create(clusterConfig_.get(), err));
  LOG_IF(FATAL, !consumer_) << err;
  return consumer;
}

std::vector<std::string> HighLevelKafkaConsumer::getTopicNames(
  const std::vector<KafkaConsumerTopicMetadata> &topicMetadata) {
  std::vector<std::string> names;
  names.resize(topicMetadata.size());
  std::transform(topicMetadata.begin(), topicMetadata.end(), names.begin(),
                 [](const auto &tMeta) { return tMeta.topicName; });
  return names;
}


std::map<std::string, std::string> HighLevelKafkaConsumer::defaultOptions(
  const std::vector<std::string> &brokers,
  const std::vector<std::string> &topicNames) {
  Random rand{};
  std::map<std::string, std::string> options{
    {"metadata.broker.list", folly::join(",", brokers)},
    {"client.id", "concord_client_id_" + std::to_string(rand.nextRand())},
    {"receive.message.max.bytes", "512000000"}, // Max receive buff or 512MB
    {"fetch.message.max.bytes", "20000"},       // Some smmalller default
    {"statistics.interval.ms", "60000"},        // every minute
  };
  if(!FLAGS_kafka_consumer_debug.empty()) {
    options.insert({"debug", FLAGS_kafka_consumer_debug});
  }
  return options;
}

void HighLevelKafkaConsumer::rebalance_cb(
  RdKafka::KafkaConsumer *consumer,
  RdKafka::ErrorCode err,
  std::vector<RdKafka::TopicPartition *> &partitions) {
  LOG(INFO) << "RdKafka::RebalanceCb. partitions: " << partitions.size();
  if(!partitions.empty()) {
    // compuate max fetch.message.max.bytes
    std::string maxBytesStr = "0";
    if(clusterConfig_->get("receive.message.max.bytes", maxBytesStr)
       != RdKafka::Conf::CONF_OK) {
      maxBytesStr = "0";
    }
    auto maxMsgBytes = std::stol(maxBytesStr);
    // our default # of partitions
    maxMsgBytes /= partitions.size();
    maxMsgBytes -= 10000;                      // Some constant for the overhead
    maxMsgBytes = std::max(512l, maxMsgBytes); // failsafe
    std::string kafkaErr;
    LOG_IF(ERROR, clusterConfig_->set("fetch.message.max.bytes",
                                      std::to_string(maxMsgBytes), kafkaErr)
                    != RdKafka::Conf::CONF_OK)
      << kafkaErr;
  }

  if(err == RdKafka::ERR__ASSIGN_PARTITIONS) {
    for(auto &p : partitions) {
      for(auto &m : topicMetadata_) {
        if(m.topicName == p->topic() && !m.offset_set) {
          // !! important: Ensure that the offset is only set once per
          // topic for this consuming group.
          m.offset_set = true;
          LOG(INFO) << "Setting the offset for: " << m.topicName
                    << ", to: " << m.startOffset << ", from: " << p->offset()
                    << ", on partition: " << p->partition();
          p->set_offset(m.startOffset);
        }
      }
    }
    LOG(INFO) << "Assigning partitions";
    consumer->assign(partitions);
  } else if(err == RdKafka::ERR__REVOKE_PARTITIONS) {
    LOG(INFO) << "Unassigning all partitions";
    consumer->unassign();
  } else {
    LOG(FATAL) << "Unknown error condition" << static_cast<int>(err);
  }
}
}
