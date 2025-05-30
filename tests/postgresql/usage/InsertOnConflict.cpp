/**
 * Copyright © 2014-2019, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <iostream>

#include <sqlpp23/postgresql/postgresql.h>
#include <sqlpp23/sqlpp23.h>

#include <sqlpp23/tests/postgresql/tables.h>
#include <sqlpp23/tests/postgresql/make_test_connection.h>
#include "sqlpp23/tests/core/result_helpers.h"

namespace sql = sqlpp::postgresql;

int InsertOnConflict(int, char*[]) {
  test::TabFoo foo = {};

  sql::connection db = sql::make_test_connection();

  test::createTabFoo(db);

  // Test on conflict
  db(sql::insert_into(foo).default_values().on_conflict().do_nothing());

  // Test on conflict (with conflict target)
  db(sql::insert_into(foo).default_values().on_conflict(foo.id).do_nothing());

  // Test on conflict (with mulitple conflict targets)
  db(sql::insert_into(foo)
         .default_values()
         .on_conflict(foo.id, dynamic(true, foo.intNnU))
         .do_nothing());
  db(sql::insert_into(foo)
         .default_values()
         .on_conflict(foo.id, dynamic(false, foo.intNnU))
         .do_nothing());

  // Conflict target
  db(sql::insert_into(foo).default_values().on_conflict(foo.id).do_update(
      foo.intN = 5, foo.textNnD = "test bla", foo.boolN = true));

  // With where statement
  for (const auto& row : db(sql::insert_into(foo)
                                .default_values()
                                .on_conflict(foo.id)
                                .do_update(foo.intN = 5,
          dynamic(true, foo.textNnD = "test bla"), foo.boolN = true)
                                .where(foo.intN == 2)
                                .returning(foo.textNnD))) {
    std::cout << row.textNnD << std::endl;
  }

  // Returning
  for (const auto& row : db(sql::insert_into(foo)
                                .default_values()
                                .on_conflict(foo.id)
                                .do_update(foo.intN = 5,
          foo.textNnD = "test bla", foo.boolN = true)
                                .returning(foo.intN))) {
    std::cout << row.intN << std::endl;
  }

  return 0;
}
