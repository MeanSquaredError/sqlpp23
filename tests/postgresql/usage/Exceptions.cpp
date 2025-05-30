/*
 * Copyright (c) 2014-2015, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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

#include <sqlpp23/core/basic/verbatim.h>
#include <sqlpp23/core/database/exception.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/postgresql/postgresql.h>

#include "assertThrow.h"

#include <sqlpp23/tests/postgresql/tables.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>

namespace sql = sqlpp::postgresql;
int Exceptions(int, char*[]) {
  {
    // broken_connection exception on bad config
    auto config = std::make_shared<sql::connection_config>();
    config->host = "non-existing-host";
    assert_throw(sql::connection db(config), sqlpp::exception);
  }

  test::TabExcept tab;
  sql::connection db = sql::make_test_connection();

  try {
    test::createTabExcept(db);
    assert_throw(db(insert_into(tab).set(
                     tab.intSmallNU = std::numeric_limits<int16_t>::max() + 1)),
                 sqlpp::exception);
    assert_throw(db(insert_into(tab).set(tab.textShortN = "123456")),
                 sqlpp::exception);
    db(insert_into(tab).set(tab.intSmallNU = 5));
    assert_throw(db(insert_into(tab).set(tab.intSmallNU = 5)),
                 sqlpp::exception);
    assert_throw(db.last_insert_id("tabfoo", "no_such_column"),
                 sqlpp::exception);

    try {
      // Cause specific error
      db(R"(create or replace function cause_error() returns int as $$
                      begin
                        raise exception 'User error' USING ERRCODE='ZX123';
                      end;
                    $$ language plpgsql
                    )");

      db("select cause_error();");
    } catch (const sqlpp::exception& e) {
      std::cout << "Caught expected error. message: " << e.what() << '\n';
    }
  } catch (const sqlpp::exception& e) {
    std::cout << e.what();
    return 1;
  }

  return 0;
}
