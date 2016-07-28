#pragma once
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <librdkafka/rdkafkacpp.h>
#include "Random.hpp"

namespace concord {
class HighLevelKafkaClient : public RdKafka::EventCb {
  public:
  HighLevelKafkaClient(std::map<std::string, std::string> &&defaultOpts,
                       const std::map<std::string, std::string> &opts = {}) {
    std::string err;
    LOG_IF(FATAL, clusterConfig_->set("event_cb", (RdKafka::EventCb *)this, err)
                    != RdKafka::Conf::CONF_OK)
      << err;
    setKafkaOptions(std::move(defaultOpts), opts, clusterConfig_.get());
  }

  virtual ~HighLevelKafkaClient() {}

  // RdKafka::EventCb methods messages from librdkafka, not from the brokers.
  void event_cb(RdKafka::Event &event) override {
    switch(event.type()) {
    case RdKafka::Event::EVENT_ERROR:
      switch(event.err()) {
      case RdKafka::ERR__ALL_BROKERS_DOWN: {
        // This is to prevent the herd effect where all the nodes on a
        // network partition would commit suicide due to kafka cluster
        // network outage and then overwhelm the clusters also exhausting shared
        // resources such as network bandwidth.
        //
        Random r;
        auto sleepTime = std::chrono::milliseconds(r.nextRand() % 1000);
        LOG(ERROR) << "All kafka brokers down. Sleeping for: "
                   << sleepTime.count() << "ms. Retries left: " << retries_;
        std::this_thread::sleep_for(sleepTime);
        --retries_;
        LOG_IF(FATAL, retries_ <= 0)
          << "ALL kafka brokers are down. Exhausted all retries. Exiting";
        break;
      }
      default:
        LOG(ERROR) << "Librdkafka error: " << RdKafka::err2str(event.err());
        break;
      }
      break;
    case RdKafka::Event::EVENT_STATS:
      LOG(INFO) << "Librdkafka stats: " << event.str();
      break;
    case RdKafka::Event::EVENT_LOG:
      LOG(INFO) << "Librdkafka log: severity: "
                << static_cast<int>(event.severity())
                << ", fac: " << event.fac() << ", event: " << event.str();
      break;
    case RdKafka::Event::EVENT_THROTTLE:
      LOG(WARNING) << "THROTTLED: " << event.throttle_time() << "ms by "
                   << event.broker_name() << " id " << event.broker_id();
      break;
    default:
      LOG(ERROR) << "Librdkafka unknown event: type: "
                 << static_cast<int>(event.type())
                 << ", str: " << RdKafka::err2str(event.err());
      break;
    }
  }

  protected:
  void resetRetryCount() { retries_ = 10; }

  private:
  void setKafkaOptions(std::map<std::string, std::string> &&defaultOpts,
                       const std::map<std::string, std::string> &opts,
                       const RdKafka::Conf *config) {
    for(auto &t : opts) {
      auto it = defaultOpts.find(t.first);
      if(it != defaultOpts.end()) {
        LOG(INFO) << "Overriding: " << t.first << " from " << it->second
                  << " -> " << t.second;
      }
      // always override
      defaultOpts.insert(t);
    }

    LOG_IF(INFO, defaultOpts.find("compression.codec") == defaultOpts.end())
      << "No kafka codec selected. Consider using compression.codec:snappy "
         "when producing and consuming";

    // set the automatic topic creation and make sure you set these partitions
    // on the kafka broker itself. On the server.properties.
    // num.partitions=144;
    std::string err;
    for(const auto &t : defaultOpts) {
      LOG(INFO) << "Kafka " << RdKafka::version_str() << " " << t.first << ":"
                << t.second;
      LOG_IF(ERROR, clusterConfig_->set(t.first, t.second, err)
                      != RdKafka::Conf::CONF_OK)
        << "Could not set variable: " << t.first << " -> " << t.second << err;
    }
  }

  protected:
  const std::unique_ptr<RdKafka::Conf> clusterConfig_{
    RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};
  int32_t retries_ = 10;
};
}
