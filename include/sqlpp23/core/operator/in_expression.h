#pragma once

/*
Copyright (c) 2024, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <tuple>
#include <vector>

#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct operator_in {
  static constexpr auto symbol = " IN";
};

struct operator_not_in {
  static constexpr auto symbol = " NOT IN";
};

template <typename L, typename Operator, typename Container>
struct in_expression : public enable_as<in_expression<L, Operator, Container>> {
  constexpr in_expression(L l, Container r)
      : _l(std::move(l)), _r(std::move(r)) {}
  in_expression(const in_expression&) = default;
  in_expression(in_expression&&) = default;
  in_expression& operator=(const in_expression&) = default;
  in_expression& operator=(in_expression&&) = default;
  ~in_expression() = default;

  L _l;
  Container _r;
};

template <typename L, typename Operator, typename... Args>
struct in_expression<L, Operator, std::tuple<Args...>>
    : public enable_as<in_expression<L, Operator, std::tuple<Args...>>> {
  constexpr in_expression(L l, std::tuple<Args...> r)
      : _l(std::move(l)), _r(std::move(r)) {}
  in_expression(const in_expression&) = default;
  in_expression(in_expression&&) = default;
  in_expression& operator=(const in_expression&) = default;
  in_expression& operator=(in_expression&&) = default;
  ~in_expression() = default;

  L _l;
  std::tuple<Args...> _r;
};

template <typename L, typename... Args>
using check_in_args = std::enable_if_t<
    (sizeof...(Args) != 0) and
    logic::all<values_are_comparable<L, Args>::value...>::value>;

template <typename L, typename Operator, typename R>
struct data_type_of<in_expression<L, Operator, std::vector<R>>>
    : std::conditional<sqlpp::is_optional<data_type_of_t<L>>::value or
                           sqlpp::is_optional<data_type_of_t<R>>::value,
                       std::optional<boolean>,
                       boolean> {};

template <typename L, typename Operator, typename... Args>
struct data_type_of<in_expression<L, Operator, std::tuple<Args...>>>
    : std::conditional<sqlpp::is_optional<data_type_of_t<L>>::value or
                           logic::any<sqlpp::is_optional<
                               data_type_of_t<Args>>::value...>::value,
                       std::optional<boolean>,
                       boolean> {};

template <typename L, typename Operator, typename R>
struct nodes_of<in_expression<L, Operator, std::vector<R>>> {
  using type = detail::type_vector<L, R>;
};

template <typename L, typename Operator, typename R>
struct requires_parentheses<in_expression<L, Operator, std::vector<R>>>
    : public std::true_type {};

template <typename L, typename Operator, typename... Args>
struct nodes_of<in_expression<L, Operator, std::tuple<Args...>>> {
  using type = detail::type_vector<L, Args...>;
};

template <typename L, typename Operator, typename... Args>
struct requires_parentheses<in_expression<L, Operator, std::tuple<Args...>>>
    : public std::true_type {};

template <typename Context, typename L, typename Operator, typename... Args>
auto to_sql_string(Context& context,
                   const in_expression<L, Operator, std::tuple<Args...>>& t)
    -> std::string {
  auto result = operand_to_sql_string(context, t._l) + Operator::symbol + " (";
  if (sizeof...(Args) == 1) {
    result += to_sql_string(context, std::get<0>(t._r));
  } else {
    result += tuple_to_sql_string(context, t._r, tuple_operand{", "});
  }
  result += ")";
  return result;
}

template <typename Container>
struct value_list_t;

template <typename Context, typename L, typename Operator, typename R>
auto to_sql_string(Context& context,
                   const in_expression<L, Operator, std::vector<R>>& t)
    -> std::string {
  if (t._r.empty()) {
    // SQL would normally treat this as a bug in the query.
    // IN requires one parameter at least.
    // But the statement "L NOT IN empty_set" is true, so let's treat this as a
    // bool result.
    return to_sql_string(context,
                         std::is_same<Operator, operator_not_in>::value);
  }
  auto result = to_sql_string(context, t._l) + Operator::symbol + " (";
  bool first = true;
  for (const auto& entry : t._r) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }

    if (t._r.size() == 1) {
      // A single entry does not need extra parentheses.
      result += to_sql_string(context, entry);
    } else {
      result += operand_to_sql_string(context, entry);
    }
  }
  result += ")";
  return result;
}

template <typename L, typename... Args, typename = check_in_args<L, Args...>>
constexpr auto in(L l, std::tuple<Args...> args)
    -> in_expression<L, operator_in, std::tuple<Args...>> {
  static_assert(sizeof...(Args) > 0, "");
  return {std::move(l), std::move(args)};
}

template <typename L, typename... Args, typename = check_in_args<L, Args...>>
constexpr auto in(L l, Args... args)
    -> in_expression<L, operator_in, std::tuple<Args...>> {
  static_assert(sizeof...(Args) > 0, "");
  return {std::move(l), std::make_tuple(std::move(args)...)};
}

template <typename L, typename Arg, typename = check_in_args<L, Arg>>
constexpr auto in(L l, std::vector<Arg> args)
    -> in_expression<L, operator_in, std::vector<Arg>> {
  return {std::move(l), std::move(args)};
}

template <typename L, typename... Args, typename = check_in_args<L, Args...>>
constexpr auto not_in(L l, std::tuple<Args...> args)
    -> in_expression<L, operator_not_in, std::tuple<Args...>> {
  return {std::move(l), std::move(args)};
}

template <typename L, typename... Args, typename = check_in_args<L, Args...>>
constexpr auto not_in(L l, Args... args)
    -> in_expression<L, operator_not_in, std::tuple<Args...>> {
  return {std::move(l), std::make_tuple(std::move(args)...)};
}

template <typename L, typename Arg, typename = check_in_args<L, Arg>>
constexpr auto not_in(L l, std::vector<Arg> args)
    -> in_expression<L, operator_not_in, std::vector<Arg>> {
  return {std::move(l), std::move(args)};
}

}  // namespace sqlpp
