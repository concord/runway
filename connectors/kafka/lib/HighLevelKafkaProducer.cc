#include "HighLevelKafkaProducer.hpp"
#include "Random.hpp"

namespace bolt {
HighLevelKafkaProducer::HighLevelKafkaProducer(
  const std::vector<std::string> &brokers,
  const std::map<std::string, std::string> &opts)
  : HighLevelKafkaClient(defaultOptions(brokers), opts)
  , producer_(buildProducer()) {
  LOG(INFO) << "Configuration: " << folly::join(" ", *clusterConfig_->dump());
}


HighLevelKafkaProducer::~HighLevelKafkaProducer() {
  auto logfun = [this]() {
    LOG(INFO) << "Bytes Sent: " << bytesSent_ << ", msgsSent: " << msgsSent_
              << ", kafka send error bytes" << bytesKafkaSendError_
              << ", kafka send error msgs: " << msgsKafkaSendError_
              << ", kafka bytes acknowledgements: " << bytesKafkaReceived_
              << ", msgs kafka acknowlege recevied " << msgsKafkaReceived_;
  };
  int32_t outq = 0;
  while((outq = producer_->outq_len()) > 0) {
    LOG(WARNING) << "Waiting 1sec to drain queue: " << outq;
    producer_->poll(1000);
    logfun();
  }
  logfun();
}

std::unique_ptr<RdKafka::Producer> HighLevelKafkaProducer::buildProducer() {
  std::string err;
  LOG_IF(FATAL, clusterConfig_->set("dr_cb", (RdKafka::DeliveryReportCb *)this,
                                    err) != RdKafka::Conf::CONF_OK)
    << err;


  auto producer = std::unique_ptr<RdKafka::Producer>(
    RdKafka::Producer::create(clusterConfig_.get(), err));
  LOG_IF(FATAL, !producer) << "Could not create producer: " << err;
  return producer;
}

void HighLevelKafkaProducer::dr_cb(RdKafka::Message &message) {
  if(message.err()) {
    LOG(ERROR) << "Kafka producer error: " << message.errstr()
               << ", topic: " << message.topic_name();
    bytesKafkaSendError_ += message.len();
    ++msgsKafkaSendError_;
  } else {
    bytesKafkaReceived_ += message.len();
    ++msgsKafkaReceived_;
  }
}

std::map<std::string, std::string> HighLevelKafkaProducer::defaultOptions(
  const std::vector<std::string> &brokers) {
  Random rand{};
  return std::map<std::string, std::string>{
    {"metadata.broker.list", folly::join(", ", brokers)},
    // 1M bytes for now. used to be a million
    {"queue.buffering.max.messages", "1000000"},
    // 50k max messages in memory
    {"batch.num.messages", "50000"},
    {"client.id", "concord_client_id_" + std::to_string(rand.nextRand())},
    {"statistics.interval.ms", "60000"} // every minute
  };
}

void HighLevelKafkaProducer::registerTopic(const std::string &topic) {
  if(topicConfigs_.find(topic) == topicConfigs_.end()) {
    LOG(INFO) << "Creating producer topic: " << topic;
    auto ptr =
      std::make_unique<HighLevelKafkaProducerTopic>(producer_.get(), topic);
    topicConfigs_.emplace(topic, std::move(ptr));
  }
}

void HighLevelKafkaProducer::produce(const std::string &topic,
                                     const std::string &key,
                                     const std::string &value,
                                     int32_t partition) {
  registerTopic(topic);
  auto &t = topicConfigs_[topic]; // reference to the unique ptr
  auto maxTries = 10;
  RdKafka::ErrorCode resp;
  while(maxTries-- > 0) {
    resp = t->producer->produce(
      t->topic.get(), partition, RdKafka::Producer::RK_MSG_COPY,
      (char *)value.c_str(), value.length(), &key, NULL);
    LOG_IF(ERROR, resp != RdKafka::ERR_NO_ERROR)
      << "Issue when producing: " << RdKafka::err2str(resp);
    if(resp == RdKafka::ERR__QUEUE_FULL) {
      producer_->poll(1);
    } else if(resp == RdKafka::ERR_NO_ERROR) {
      maxTries = 0;
    }
  }
  if(resp == RdKafka::ERR_NO_ERROR) {
    resetRetryCount();
  }
  if(resp != RdKafka::ERR_NO_ERROR) {
    LOG(ERROR) << "After 10 tries, skipping message due to: "
               << RdKafka::err2str(resp);
  }
  bytesSent_ += value.length() + key.length();
  ++msgsSent_;
  if(msgsSent_ % 10000 == 0) {
    producer_->poll(1);
    LOG(INFO) << "Total msgs sent: " << msgsSent_
              << ", total bytes sent: " << bytesSent_
              << ", bytes received by the broker: " << bytesKafkaReceived_
              << ", msgs received by broker: " << msgsKafkaReceived_
              << ", error bytes attempted to send: " << bytesKafkaSendError_
              << ", error msgs sent to broker: " << msgsKafkaSendError_;
  }
}
}
