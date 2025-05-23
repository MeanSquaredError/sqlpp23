/*
 * Copyright (c) 2025, Roland Bock
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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

// Returns true if `case_when(declcal<Lhs>)` is a valid function call.
template <typename Lhs, typename = void>
struct can_call_case_when_with : public std::false_type {};

template <typename Lhs>
struct can_call_case_when_with<
    Lhs,
    std::void_t<decltype(sqlpp::case_when(std::declval<Lhs>()))>>
    : public std::true_type {};

// Returns true if `declcal<Lhs>.then(declval<Rhs>())` is a valid function call.
template <typename Lhs, typename Rhs, typename = void>
struct can_call_then_with : public std::false_type {};

template <typename Lhs, typename Rhs>
struct can_call_then_with<
    Lhs,
    Rhs,
    std::void_t<decltype(std::declval<Lhs>().then(std::declval<Rhs>()))>>
    : public std::true_type {};

// Returns true if `declcal<Lhs>.else_(declval<Rhs>())` is a valid function
// call.
template <typename Lhs, typename Rhs, typename = void>
struct can_call_else_with : public std::false_type {};

template <typename Lhs, typename Rhs>
struct can_call_else_with<
    Lhs,
    Rhs,
    std::void_t<decltype(std::declval<Lhs>().else_(std::declval<Rhs>()))>>
    : public std::true_type {};
}  // namespace

int main() {
  const auto maybe = true;
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // -----------------------
  // case_when()
  // -----------------------

  // OK
  static_assert(can_call_case_when_with<decltype(true)>::value, "");
  static_assert(
      can_call_case_when_with<decltype(std::make_optional(true))>::value, "");
  static_assert(can_call_case_when_with<decltype(foo.boolN)>::value, "");
  static_assert(can_call_case_when_with<decltype(bar.boolNn)>::value, "");
  static_assert(can_call_case_when_with<decltype(bar.boolNn == true)>::value,
                "");
  static_assert(can_call_case_when_with<decltype(count(foo.id) > 0)>::value,
                "");
  static_assert(can_call_case_when_with<decltype(std::nullopt)>::value, "");

  // Fail: Cannot call case_when with renamed boolean
  static_assert(
      not can_call_case_when_with<decltype(bar.boolNn.as(something))>::value,
      "");

  // Fail: Cannot call case_when with non-boolean expressions.
  static_assert(not can_call_case_when_with<decltype(bar.id)>::value, "");
  static_assert(not can_call_case_when_with<decltype(bar.boolNn = true)>::value,
                "");
  static_assert(not can_call_case_when_with<decltype(bar)>::value, "");

  // -----------------------
  // case_when.then()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_then_with<CW, decltype(bar.id)>::value, "");
    static_assert(can_call_then_with<CW, decltype(bar.textN)>::value, "");
    static_assert(
        can_call_then_with<CW,
                           decltype(std::optional<int>(std::nullopt))>::value,
        "");

    // Fail: Cannot use nullopt, as we need a data_type for the CASE
    // expression.
    static_assert(not can_call_then_with<CW, decltype(std::nullopt)>::value,
                  "");

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_then_with<CW, decltype(bar.boolNn = true)>::value, "");
    static_assert(
        not can_call_then_with<CW, decltype(bar.boolNn.as(something))>::value,
        "");
    static_assert(not can_call_then_with<CW, decltype(bar)>::value, "");
  }

  // -----------------------
  // case_when.then(<nullable text>).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(bar.textN);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>::value, "");
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>::value, "");
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>::value,
                  "");

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>::value, "");

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>::value, "");
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>::value,
        "");
    static_assert(not can_call_else_with<CW, decltype(bar)>::value, "");
  }

  // -----------------------
  // case_when.then(<non-nullable text>).else_()
  // -----------------------
  {
    auto cw = sqlpp::case_when(maybe).then(foo.textNnD);
    using CW = decltype(cw);

    // OK
    static_assert(can_call_else_with<CW, decltype(bar.textN)>::value, "");
    static_assert(can_call_else_with<CW, decltype(foo.textNnD)>::value, "");
    static_assert(can_call_else_with<CW, decltype(std::optional<std::string>(
                                             std::nullopt))>::value,
                  "");

    // OK: the value type of CASE is determined by the THEN expression.
    static_assert(can_call_else_with<CW, decltype(std::nullopt)>::value, "");

    // Fail: Anything that does not have a value.
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn = true)>::value, "");
    static_assert(
        not can_call_else_with<CW, decltype(bar.boolNn.as(something))>::value,
        "");
    static_assert(not can_call_else_with<CW, decltype(bar)>::value, "");
  }
}
