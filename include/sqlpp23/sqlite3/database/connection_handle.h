#pragma once

/*
 * Copyright (c) 2013 - 2016, Roland Bock
 * Copyright (c) 2023, Vesselin Atanasov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <memory>

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

#include <sqlpp23/sqlite3/database/connection_config.h>
#include <sqlpp23/sqlite3/database/exception.h>

namespace sqlpp::sqlite3::detail {
struct connection_handle {
  std::shared_ptr<const connection_config> config;
  std::unique_ptr<::sqlite3, int (*)(::sqlite3*)> sqlite;

  connection_handle()
      : config{}, sqlite{nullptr, sqlite3_close} {}

  connection_handle(const std::shared_ptr<const connection_config>& conf)
      : config{conf}, sqlite{nullptr, sqlite3_close} {
    {
      ::sqlite3* sqlite_ptr;
      const auto rc = sqlite3_open_v2(
          conf->path_to_database.c_str(), &sqlite_ptr, conf->flags,
          conf->vfs.empty() ? nullptr : conf->vfs.c_str());
      sqlite.reset(sqlite_ptr);
      if (rc != SQLITE_OK) {
        throw exception{sqlite3_errmsg(native_handle()), rc};
      }
    }
    if (config->use_extended_result_codes) {
      if (const auto rc =
              sqlite3_extended_result_codes(native_handle(), true)) {
        throw exception{sqlite3_errmsg(native_handle()), rc};
      }
    }

#ifdef SQLITE_HAS_CODEC
    if (conf->password.size() > 0) {
      int ret = sqlite3_key(native_handle(), conf->password.data(),
                            conf->password.size());
      if (ret != SQLITE_OK) {
        throw exception{sqlite3_errmsg(native_handle()), rc};
      }
    }
#endif
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&&) = default;
  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&&) = default;

  ::sqlite3* native_handle() const { return sqlite.get(); }

  bool is_connected() const {
    // The connection is established in the constructor and the SQLite3 client
    // library doesn't seem to have a way to check passively if the connection
    // is still valid
    return native_handle() != nullptr;
  }

  bool ping_server() const {
    // Loosely based on the implementation of PHP's pg_ping()
    return is_connected() and
           (sqlite3_exec(native_handle(), "SELECT 1", nullptr, nullptr,
                         nullptr) == SQLITE_OK);
  }

  const debug_logger& debug() { return config->debug; }
};
}  // namespace sqlpp::sqlite3::detail
