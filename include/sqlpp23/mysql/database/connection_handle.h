#pragma once

/*
 * Copyright (c) 2013 - 2017, Roland Bock
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

#include <sqlpp23/mysql/database/connection_config.h>
#include <sqlpp23/mysql/database/exception.h>
#include <sqlpp23/mysql/sqlpp_mysql.h>

namespace sqlpp::mysql::detail {
inline void connect(MYSQL* mysql, const connection_config& config) {
  if (config.connect_timeout_seconds != 0 &&
      mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                    &config.connect_timeout_seconds)) {
    throw exception{mysql_error(mysql), mysql_errno(mysql)};
  }

  if (config.read_timeout > 0 &&
      mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &config.read_timeout)) {
    throw exception{mysql_error(mysql), mysql_errno(mysql)};
  }

  if (config.ssl) {
    if (!config.ssl_key.empty() &&
        mysql_options(mysql, MYSQL_OPT_SSL_KEY, config.ssl_key.c_str())) {
      throw exception{mysql_error(mysql), mysql_errno(mysql)};
    }
    if (!config.ssl_cert.empty() &&
        mysql_options(mysql, MYSQL_OPT_SSL_CERT, config.ssl_cert.c_str())) {
      throw exception{mysql_error(mysql), mysql_errno(mysql)};
    }
    if (!config.ssl_ca.empty() &&
        mysql_options(mysql, MYSQL_OPT_SSL_CA, config.ssl_ca.c_str())) {
      throw exception{mysql_error(mysql), mysql_errno(mysql)};
    }
    if (!config.ssl_capath.empty() &&
        mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, config.ssl_capath.c_str())) {
      throw exception{mysql_error(mysql), mysql_errno(mysql)};
    }
    if (!config.ssl_cipher.empty() &&
        mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, config.ssl_cipher.c_str())) {
      throw exception{mysql_error(mysql), mysql_errno(mysql)};
    }
  }

  if (!mysql_real_connect(
          mysql, config.host.empty() ? nullptr : config.host.c_str(),
          config.user.empty() ? nullptr : config.user.c_str(),
          config.password.empty() ? nullptr : config.password.c_str(), nullptr,
          config.port,
          config.unix_socket.empty() ? nullptr : config.unix_socket.c_str(),
          config.client_flag)) {
    throw exception{mysql_error(mysql), mysql_errno(mysql)};
  }

  if (mysql_set_character_set(mysql, config.charset.c_str())) {
    throw exception{mysql_error(mysql), mysql_errno(mysql)};
  }

  if (not config.database.empty() and
      mysql_select_db(mysql, config.database.c_str())) {
    throw exception{mysql_error(mysql), mysql_errno(mysql)};
  }
}

struct connection_handle {
  std::shared_ptr<const connection_config> config;
  std::unique_ptr<MYSQL, void (*)(MYSQL*)> mysql;

  connection_handle() : config{}, mysql{nullptr, mysql_close} {}

  connection_handle(const std::shared_ptr<const connection_config>& conf)
      : config{conf}, mysql{mysql_init(nullptr), mysql_close} {
    if (not mysql) {
      throw sqlpp::exception{"MySQL: could not init mysql data structure"};
    }

    connect(native_handle(), *config);
  }

  connection_handle(const connection_handle&) = delete;
  connection_handle(connection_handle&&) = default;
  connection_handle& operator=(const connection_handle&) = delete;
  connection_handle& operator=(connection_handle&&) = default;

  MYSQL* native_handle() const { return mysql.get(); }

  bool is_connected() const {
    // The connection is established in the constructor and the MySQL client
    // library doesn't seem to have a way to check passively if the connection
    // is still valid
    return native_handle() != nullptr;
  }

  bool ping_server() const {
    return native_handle() and (mysql_ping(native_handle()) == 0);
  }

  const debug_logger& debug() { return config->debug; }
};
}  // namespace sqlpp::mysql::detail
