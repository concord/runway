#include "CassandraInserter.hpp"
namespace concord {
// Utility methods and aliases
using CassFuturePtr = std::unique_ptr<CassFuture, decltype(&cass_future_free)>;

CassFuturePtr wrapCassFuture(CassFuture *f) {
  return CassFuturePtr(f, cass_future_free);
}

std::string cassFutureErrorString(CassFuture *f) {
  const char *message;
  size_t message_length;
  cass_future_error_message(f, &message, &message_length);
  return message;
}

// CassandraInserter public methods:
CassandraInserter::CassandraInserter(const std::string &keyspace,
                                     const std::string &table,
                                     const std::string &contactPoints)
    : keyspace_(keyspace), tableName_(table) {
  cass_cluster_set_contact_points(cluster_, contactPoints.c_str());
}

CassandraInserter::CassandraInserter(CassandraInserter &&other)
    : keyspace_(std::move(other.keyspace_)),
      tableName_(std::move(other.tableName_)),
      tableSchema_(std::move(other.tableSchema_)), session_(other.session_),
      cluster_(other.cluster_) {
  other.session_ = nullptr;
  other.cluster_ = nullptr;
}

CassandraInserter::~CassandraInserter() {
  cass_cluster_free(cluster_);
  cass_session_free(session_);
  cluster_ = nullptr;
  session_ = nullptr;
}

folly::Future<folly::Optional<std::string>>
CassandraInserter::connectSession() {
  LOG(INFO) << "Establishing connection with Cassandra...";  
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

// CassandraInserter private methods:
folly::Optional<std::string> CassandraInserter::setSchemaMetadata() {
  if (const auto cMeta = queryTableMetadata()) {
    tableSchema_ = *cMeta;
    return folly::none;
  }
  return folly::Optional<std::string>("Database returned no metadata");
}

folly::Optional<std::vector<std::string>>
CassandraInserter::queryTableMetadata() {
  const CassSchemaMeta *schema_meta = cass_session_get_schema_meta(session_);
  const CassKeyspaceMeta *keyspace_meta =
      cass_schema_meta_keyspace_by_name(schema_meta, keyspace_.c_str());

  if (keyspace_meta == NULL) {
    return folly::none;
  }

  const CassTableMeta *table_meta =
      cass_keyspace_meta_table_by_name(keyspace_meta, tableName_.c_str());

  std::vector<std::string> md;
  CassIterator *iterator = cass_iterator_columns_from_table_meta(table_meta);
  while (cass_iterator_next(iterator)) {
    const CassColumnMeta *colMeta = cass_iterator_get_column_meta(iterator);
    const char *name;
    size_t name_length;
    cass_column_meta_name(colMeta, &name, &name_length);
    md.emplace_back(name);
  }
  cass_iterator_free(iterator);
  cass_schema_meta_free(schema_meta);
  return folly::Optional<std::vector<std::string>>(md);
}

folly::Future<bool> CassandraInserter::registerCallback(CassFuture *ft) {
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
  LOG_IF(ERROR, !success) << "Asynchronous Event Error: "
                          << cassFutureErrorString(future.get());
  promise->setValue(success);
}
}
