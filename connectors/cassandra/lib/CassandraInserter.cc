#include "CassandraInserter.hpp"
#include "CassandraUtils.hpp"
namespace concord {
CassandraInserter::CassandraInserter(const std::string &keyspace,
                                     const std::string &table,
                                     const std::string &contactPoints)
    : keyspace_(keyspace), tableName_(table),
      cluster_(createCluster(contactPoints)) {}

CassandraInserter::~CassandraInserter() {
  cass_cluster_free(cluster_);
  cass_session_free(session_);
}

folly::Future<folly::Optional<std::string>>
CassandraInserter::connectSession() {
  return registerCallback(cass_session_connect(session_, cluster_))
      .then([this](bool success) {
        if (success) {
          LOG(INFO) << "Successful connection established";
          return setSchemaMetadata();
        } else {
          return folly::Optional<std::string>("Failed to connect to Cassandra");
        }
      });
}

void CassandraInserter::disconnectSession() {
  LOG(INFO) << "Closing connection to Cassandra";
  auto close = wrapCassFuture(cass_session_close(session_));
  cass_future_wait(close.get());
}

// Private methods
folly::Optional<std::string> CassandraInserter::setSchemaMetadata() {
  if (const auto cMeta =
          queryTableMetadata(session_, keyspace_.c_str(), tableName_.c_str())) {
    tableSchema_ = *cMeta;
    return folly::none;
  }
  return folly::Optional<std::string>("Database returned no metadata");
}

folly::Future<bool> CassandraInserter::registerCallback(CassFuture *ft) {
  LOG(INFO) << "Establishing connection with Cassandra...";
  auto f = wrapCassFuture(ft);
  auto promise = std::make_unique<folly::Promise<bool>>();
  auto future = promise->getFuture();
  if (cass_future_set_callback(f.release(), databaseEffectCallback,
                               (void *)promise.release()) != CASS_OK) {
    return folly::Future<bool>(false);
  }
  return future;
}

// static
void CassandraInserter::databaseEffectCallback(CassFuture *ft, void *data) {
  auto promise =
      std::unique_ptr<folly::Promise<bool>>((folly::Promise<bool> *)data);
  auto future = wrapCassFuture(ft);
  CHECK(cass_future_ready(future.get()) == true) << "Future must be ready here";
  const bool success = cass_future_error_code(future.get()) == CASS_OK;
  promise->setValue(success);
}

CassCluster *
CassandraInserter::createCluster(const std::string &contactPoints) {
  CassCluster *cluster = cass_cluster_new();
  cass_cluster_set_contact_points(cluster, contactPoints.c_str());
  return cluster;
}
}
