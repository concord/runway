#pragma once
#include <vector>
#include <cassandra.h>
#include <folly/futures/Future.h>
#include <folly/Format.h>
#include <folly/Optional.h>
namespace concord {
class CassandraInserter {
public:
  CassandraInserter(const std::string &keyspace, const std::string &table,
                    const std::string &contactPoints);
  CassandraInserter(const CassandraInserter &) = delete;
  CassandraInserter &operator=(const CassandraInserter &) = delete;
  CassandraInserter(CassandraInserter &&other);
  // TODO: Implement this method
  CassandraInserter &operator=(const CassandraInserter &&rhs) = delete;
  virtual ~CassandraInserter();

  folly::Future<folly::Optional<std::string>> connectSession();
  void disconnectSession();

  template <typename Func>
  folly::Future<folly::Unit> insert(Func &&insertCallback) {
    // NOTE: .asString() returns fbstring, no need to convert to std::string
    std::vector<folly::fbstring> values;
    for (const auto &col : tableSchema_) {
      values.emplace_back(std::forward<Func>(insertCallback)(col));
    }

    const std::string query = folly::sformat(
        "INSERT INTO {}.{} ({}) VALUES ({});", keyspace_, tableName_,
        folly::join(",", tableSchema_), folly::join(",", values));

    return registerCallback(
        cass_session_execute(session_, cass_statement_new(query.c_str(), 0)));
  }

private:
  folly::Optional<std::string> setSchemaMetadata();
  folly::Optional<std::vector<std::string>> queryTableMetadata();
  folly::Future<folly::Unit> registerCallback(CassFuture *ft);
  static void databaseEffectCallback(CassFuture *ft, void *data);

  const std::string keyspace_;
  const std::string tableName_;
  std::vector<std::string> tableSchema_;
  CassSession *session_{cass_session_new()};
  CassCluster *cluster_{cass_cluster_new()};
};
}
