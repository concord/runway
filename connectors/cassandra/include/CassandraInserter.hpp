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
  // TODO: Implement move assignment and copy operations
  // CassandraInserter(CassandraInserter &&other);
  // CassandraInserter &operator=(const CassandraInserter &&rhs);
  virtual ~CassandraInserter();

  folly::Future<folly::Optional<std::string>> connectSession();
  void disconnectSession();

  template <typename Func> folly::Future<bool> insert(Func &&insertCallback) {
    // Example:
    // INSERT INTO tablename (COL1, ..., COLN) VALUES (data1, ... , dataN);
    //
    std::vector<const char *> values;
    for (const auto &col : tableSchema_) {
      values.push_back(insertCallback(col));
    }

    const std::string query = folly::sformat(
        "INSERT INTO {}.{} ({}) VALUES ({});", keyspace_, tableName_,
        folly::join(",", tableSchema_), folly::join(",", values));

    return registerCallback(
        cass_session_execute(session_, cass_statement_new(query.c_str(), 0)));
  }

private:
  folly::Optional<std::string> setSchemaMetadata();
  folly::Future<bool> registerCallback(CassFuture *ft);
  static void databaseEffectCallback(CassFuture *ft, void *data);
  CassCluster *createCluster(const std::string &contactPoints);

  const std::string keyspace_;
  const std::string tableName_;
  std::vector<std::string> tableSchema_;
  CassSession *session_{cass_session_new()};
  CassCluster *cluster_;
};
}
