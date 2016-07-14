#pragma once
#include <vector>
#include <cassandra.h>
#include <folly/Optional.h>
namespace concord {
using CassFuturePtr = std::unique_ptr<CassFuture, decltype(&cass_future_free)>;

CassFuturePtr wrapCassFuture(CassFuture *f) {
  return CassFuturePtr(f, cass_future_free);
}

folly::Optional<std::vector<std::string>>
queryTableMetadata(CassSession *session, const char *keyspace,
                   const char *tableName) {
  const CassSchemaMeta *schema_meta = cass_session_get_schema_meta(session);
  const CassKeyspaceMeta *keyspace_meta =
    cass_schema_meta_keyspace_by_name(schema_meta, keyspace);

  if (keyspace_meta == NULL) {
    return folly::none;
  }

  const CassTableMeta *table_meta =
      cass_keyspace_meta_table_by_name(keyspace_meta, tableName);

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
}
