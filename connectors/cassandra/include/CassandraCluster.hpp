#pragma once
#include <cassandra.h>
#include <folly/futures/Future.h>
#include <folly/Format.h>
#include "CassandraTypes.hpp"

namespace concord {

CassFuturePtr wrapCassFuture(CassFuture *);

class CassandraCluster {
public:
  CassandraCluster(const std::string &contactPoints);
  CassandraCluster(const CassandraCluster &) = delete;
  virtual ~CassandraCluster();

  folly::Future<bool> connectSession(const std::string &keyspace,
                                     const std::string &tableName);
  // void closeSession();

  template <typename... Args> folly::Future<bool> insert(Args &&... args) {
    // Example:
    // INSERT INTO tablename (COL1, ..., COLN) VALUES (data1, ... , dataN);
    //
    const std::string query =
        folly::sformat("INSERT INTO {} ({}) VALUES ({});", tableContext_,
                       folly::join(",", std::forward(args)...),
                       folly::join(",", std::forward(args)...));
    return registerCallback(
        cass_session_execute(session_, cass_statement_new(query.c_str(), 0)));
  }

private:
  static void databaseEffectCallback(CassFuture *ft, void *data);
  folly::Future<bool> registerCallback(CassFuture&& *ft);
  CassCluster *createCluster(const std::string &contactPoints);

  CassSession *session_{cass_session_new()};
  CassCluster *cluster_;
  std::string tableContext_;
};
}
