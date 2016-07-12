#pragma once
#include <cassandra.h>
#include <folly/futures/Future.h>

// typedef void(* CassFutureCallback) (CassFuture *future, void *data)

namespace concord {

// template <typename... Args> class CassCtx {
// public:
//   CassandraFutureBase(Args... args)
//       : future_(cass_future_create(std::forward<Args>(args)...)) {}
//   CassandraFutureBase(const CassandraFutureBase &) = delete;
//   virtual ~CassandraFutureBase() { cass_future_free(future_); }
//   bool isReady() const { return static_cast<bool>(cass_future_ready(future_)); }

// protected:
//   const CassFuture *future_;
// };

// template <typename T, typename... Args>
// class CassandraFuture : public CassandraFutureBase<Args> {};

// template <typename... Args>
// class CassandraFuture<bool, Args> : public CassandraFutureBase<Args> {
// public:
//   // Get waits, then returns a value
//   bool get() const { return cass_future_error_code(future_) == CASS_OK; }
// };

// template <typename... Args>
// class CassandraFuture<int, Args> : public CassandraFutureBase<Args> {
// public:
//   std::vector<T> get() const { return std::vector<T>(); }
// };
}
