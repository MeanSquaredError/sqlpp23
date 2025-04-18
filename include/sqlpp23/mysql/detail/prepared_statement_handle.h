#pragma once

/*
 * Copyright (c) 2013 - 2015, Roland Bock
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

#include <sqlpp23/mysql/sqlpp_mysql.h>

#include <vector>

namespace sqlpp::mysql::detail {
struct bind_result_buffer {
  unsigned long length;
  my_bool is_null;
  my_bool error;
  union  // unnamed union injects members into scope
  {
    bool _bool;
    int64_t _int64;
    uint64_t _uint64;
    double _double;
    MYSQL_TIME _mysql_time;
  };
  std::vector<char> var_buffer;  // text and blobs
};

struct prepared_statement_handle_t {
  struct wrapped_bool {
    my_bool value;

    wrapped_bool() : value{false} {}
    wrapped_bool(bool v) : value{v} {}
    wrapped_bool(const wrapped_bool&) = default;
    wrapped_bool(wrapped_bool&&) = default;
    wrapped_bool& operator=(const wrapped_bool&) = default;
    wrapped_bool& operator=(wrapped_bool&&) = default;
    ~wrapped_bool() = default;
  };

  MYSQL_STMT* mysql_stmt;
  std::vector<MYSQL_BIND> stmt_params;
  std::vector<MYSQL_TIME> stmt_date_time_param_buffer;
  std::vector<wrapped_bool>
      stmt_param_is_null;  // my_bool is bool after 8.0, and vector<bool> is bad
  std::vector<MYSQL_BIND> result_params;
  std::vector<bind_result_buffer> result_buffers;
  bool debug;

  prepared_statement_handle_t(MYSQL_STMT* stmt,
                              size_t no_of_parameters,
                              size_t no_of_columns,
                              bool debug_)
      : mysql_stmt(stmt),
        stmt_params(no_of_parameters,
                    MYSQL_BIND{}),  // ()-init for correct constructor
        stmt_date_time_param_buffer(
            no_of_parameters,
            MYSQL_TIME{}),  // ()-init for correct constructor
        stmt_param_is_null(no_of_parameters,
                           false),  // ()-init for correct constructor
        result_params(no_of_columns,
                      MYSQL_BIND{}),  // ()-init for correct constructor
        result_buffers(
            no_of_columns,
            bind_result_buffer{}),  // ()-init for correct constructor
        debug{debug_} {}

  prepared_statement_handle_t(const prepared_statement_handle_t&) = delete;
  prepared_statement_handle_t(prepared_statement_handle_t&&) = default;
  prepared_statement_handle_t& operator=(const prepared_statement_handle_t&) =
      delete;
  prepared_statement_handle_t& operator=(prepared_statement_handle_t&&) =
      default;

  ~prepared_statement_handle_t() {
    if (mysql_stmt)
      mysql_stmt_close(mysql_stmt);
  }

  bool operator!() const { return !mysql_stmt; }
};
}  // namespace sqlpp::mysql::detail
