/**
 * Copyright © 2017 Volker Aßmann
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

namespace {
template <typename L, typename R>
void require_equal(int line, const L& l, const R& r) {
  if (l != r) {
    std::cerr << line << ": --" << l << " != " << r << "--" << std::endl;
    throw std::runtime_error("Unexpected result");
  }
}
}  // namespace

namespace sql = sqlpp::postgresql;

int Type(int, char*[]) {
  sql::connection db = sql::make_test_connection();

  try {
    test::createTabFoo(db);
    test::createTabBar(db);

    const auto tab = test::TabBar{};
    db(insert_into(tab).default_values());
    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.intN.has_value(), false);
      require_equal(__LINE__, row.textN.has_value(), false);
      require_equal(__LINE__, row.boolNn, false);
    }

    db(update(tab).set(tab.intN = 10, tab.textN = "Cookies!",
                       tab.boolNn = true));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.intN.has_value(), true);
      require_equal(__LINE__, row.intN.value(), 10);
      require_equal(__LINE__, row.textN.has_value(), true);
      require_equal(__LINE__, row.textN.value(), "Cookies!");
      require_equal(__LINE__, row.boolNn, true);
    }

    db(update(tab).set(tab.intN = 20, tab.textN = "Monster",
                       tab.boolNn = false));

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.intN.value(), 20);
      require_equal(__LINE__, row.textN.value(), "Monster");
      require_equal(__LINE__, row.boolNn, false);
    }

    auto prepared_update = db.prepare(update(tab).set(
        tab.intN = parameter(tab.intN), tab.textN = parameter(tab.textN),
        tab.boolNn = parameter(tab.boolNn)));
    prepared_update.parameters.intN = 30;
    prepared_update.parameters.textN = "IceCream";
    prepared_update.parameters.boolNn = true;
    std::cout << "---- running prepared update ----" << std::endl;
    db(prepared_update);
    std::cout << "---- finished prepared update ----" << std::endl;

    for (const auto& row : db(select(all_of(tab)).from(tab))) {
      require_equal(__LINE__, row.intN.value(), 30);
      require_equal(__LINE__, row.textN.value(), "IceCream");
      require_equal(__LINE__, row.boolNn, true);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    return 1;
  }
  return 0;
}
