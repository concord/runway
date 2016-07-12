#include "CassandraCluster.hpp"
namespace concord {

CassFuturePtr wrapCassFuture(CassFuture *f) {
  return CassFuturePtr(f, cass_future_free);
}

CassandraCluster::CassandraCluster(const std::string &contactPoints)
    : cluster_(createCluster(contactPoints)) {}

CassandraCluster::~CassandraCluster() {
  cass_cluster_free(cluster_);
  cass_session_free(session_);
}

folly::Future<bool>
CassandraCluster::connectSession(const std::string &keyspace,
                                 const std::string &tableName) {
  tableContext_ = folly::sformat("{}.{}", keyspace, tableName);
  return registerCallback(cass_session_connect(session_, cluster_));
}

folly::Future<bool> CassandraCluster::registerCallback(CassFuture *ft) {
  auto f = wrapCassFuture(ft);
  auto promise = std::make_unique<folly::Promise<bool>>();
  auto future = promise->getFuture();
  if (cass_future_set_callback(f.release(), databaseEffectCallback,
                               (void *)promise.release()) != CASS_OK) {
    return folly::Future<bool>(false);
  }
  return future;
}

void CassandraCluster::databaseEffectCallback(CassFuture *ft, void *data) {
  auto promise =
      std::unique_ptr<folly::Promise<bool>>((folly::Promise<bool> *)data);
  auto future = wrapCassFuture(ft);
  CHECK(cass_future_ready(future.get()) == true) << "Future must be ready here";
  const bool success = cass_future_error_code(future.get()) == CASS_OK;
  promise->setValue(success);
}

CassCluster *CassandraCluster::createCluster(const std::string &contactPoints) {
  CassCluster *cluster = cass_cluster_new();
  cass_cluster_set_contact_points(cluster, contactPoints.c_str());
  return cluster;
}
}
