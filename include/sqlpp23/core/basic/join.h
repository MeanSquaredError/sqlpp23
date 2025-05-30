#pragma once

/*
 * Copyright (c) 2024, Roland Bock
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

#include <sqlpp23/core/basic/enable_join.h>
#include <sqlpp23/core/basic/join_fwd.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
// Join representation including condition
template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct join_t : enable_join {
  join_t(Lhs lhs, Rhs rhs, Condition condition)
      : _lhs(std::move(lhs)),
        _rhs(std::move(rhs)),
        _condition(std::move(condition)) {}

  join_t(const join_t&) = default;
  join_t(join_t&&) = default;
  join_t& operator=(const join_t&) = default;
  join_t& operator=(join_t&&) = default;
  ~join_t() = default;

  Lhs _lhs;
  Rhs _rhs;
  Condition _condition;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct nodes_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = sqlpp::detail::type_vector<Lhs, Rhs, Condition>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct provided_tables_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = detail::make_joined_set_t<provided_tables_of_t<Lhs>,
                                         provided_tables_of_t<Rhs>>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct provided_static_tables_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = detail::make_joined_set_t<provided_static_tables_of_t<Lhs>,
                                         provided_static_tables_of_t<Rhs>>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct provided_optional_tables_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = detail::make_joined_set_t<
      std::conditional_t<
          detail::type_vector<right_outer_join_t,
                              full_outer_join_t>::contains<JoinType>::value,
          provided_tables_of_t<Lhs>,
          provided_optional_tables_of_t<Lhs>>,
      std::conditional_t<
          detail::type_vector<left_outer_join_t,
                              full_outer_join_t>::contains<JoinType>::value,
          provided_tables_of_t<Rhs>,
          provided_optional_tables_of_t<Rhs>>>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct required_tables_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = detail::type_set<>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct required_static_tables_of<join_t<Lhs, JoinType, Rhs, Condition>> {
  using type = detail::type_set<>;
};

template <typename Lhs, typename JoinType, typename Rhs, typename Condition>
struct is_table<join_t<Lhs, JoinType, Rhs, Condition>> : public std::true_type {
};

template <typename Context,
          typename Lhs,
          typename JoinType,
          typename Rhs,
          typename Condition>
auto to_sql_string(Context& context,
                   const join_t<Lhs, JoinType, Rhs, Condition>& t)
    -> std::string {
  static_assert(not std::is_same<JoinType, cross_join_t>::value);

  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = to_sql_string(context, t._lhs) + JoinType::_name;
  ret_val += to_sql_string(context, t._rhs) + " ON ";
  return ret_val + to_sql_string(context, t._condition);
}

template <typename Context,
          typename Lhs,
          typename JoinType,
          typename Rhs,
          typename Condition>
auto to_sql_string(Context& context,
                   const join_t<Lhs, JoinType, dynamic_t<Rhs>, Condition>& t)
    -> std::string {
  static_assert(not std::is_same<JoinType, cross_join_t>::value);
  if (t._rhs.has_value()) {
    // Note: Temporary required to enforce parameter ordering.
    auto ret_val = to_sql_string(context, t._lhs) + JoinType::_name;
    ret_val += to_sql_string(context, t._rhs.value()) + " ON ";
    return ret_val + to_sql_string(context, t._condition);
  }
  return to_sql_string(context, t._lhs);
}

template <typename Context, typename Lhs, typename Rhs>
auto to_sql_string(Context& context,
                   const join_t<Lhs, cross_join_t, Rhs, unconditional_t>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = to_sql_string(context, t._lhs) + cross_join_t::_name;
  return ret_val + to_sql_string(context, t._rhs);
}

template <typename Context, typename Lhs, typename Rhs>
auto to_sql_string(
    Context& context,
    const join_t<Lhs, cross_join_t, dynamic_t<Rhs>, unconditional_t>& t)
    -> std::string {
  if (t._rhs.has_value()) {
    // Note: Temporary required to enforce parameter ordering.
    auto ret_val = to_sql_string(context, t._lhs) + cross_join_t::_name;
    return ret_val + to_sql_string(context, t._rhs.value());
  }
  return to_sql_string(context, t._lhs);
}

template <typename Lhs, typename JoinType, typename Rhs>
class pre_join_t {
  public:

  pre_join_t(Lhs lhs, Rhs rhs) : _lhs(std::move(lhs)), _rhs(std::move(rhs)) {}
  pre_join_t(const pre_join_t&) = default;
  pre_join_t(pre_join_t&&) = default;
  pre_join_t& operator=(const pre_join_t&) = default;
  pre_join_t& operator=(pre_join_t&&) = default;
  ~pre_join_t() = default;

  // on() verifies that all tables required by ON are provided by the JOIN.
  // Good examples:
  //
  // * `foo.join(dynamic(true, bar))
  //       .on(foo.id == bar.id)`
  //    The ON condition statically requires `bar`. That is OK since the
  //    condition is evaluated only in case bar is actually joined.
  // * `foo.cross_join(dynamic(true, bar))
  //       .join(cheese)
  //       .on(foo.id == cheese.id and dynamic(true, bar.id == cheese.id))`
  //    The ON condition for joining foo and (maybe) bar dynamically requires
  //    `bar`.
  //
  // Bad examples:
  //
  // * `foo.join(dynamic(true, bar))
  //       .on(foo.id == cheese.id)`
  //    `cheese` must not be used in the ON condition as it is not part of the
  //    join at all.
  // * `foo.cross_join(dynamic(true, bar))
  //       .join(cheese)
  //       .on(bar.id == cheese.id))`
  //   `bar` is dynamically joined only. It must not be used statically when
  //   joining cheese `statically`.
  template <StaticBoolean Expr>
    requires(provided_tables_of_t<pre_join_t>::contains_all(
                 required_tables_of_t<Expr>{}) and
             (is_dynamic<Rhs>::value or
              provided_static_tables_of_t<pre_join_t>::contains_all(
                  required_static_tables_of_t<Expr>{})))
  auto on(Expr expr) const -> join_t<Lhs, JoinType, Rhs, Expr> {
    return {_lhs, _rhs, std::move(expr)};
  }

 private:
  Lhs _lhs;
  Rhs _rhs;
};

template <typename Lhs, typename JoinType, typename Rhs>
struct provided_tables_of<pre_join_t<Lhs, JoinType, Rhs>>
    : public provided_tables_of<join_t<Lhs, JoinType, Rhs, bool>> {};

template <typename Lhs, typename JoinType, typename Rhs>
struct provided_static_tables_of<pre_join_t<Lhs, JoinType, Rhs>>
    : public provided_static_tables_of<join_t<Lhs, JoinType, Rhs, bool>> {};

template <typename Lhs, typename JoinType, typename Rhs>
struct is_pre_join<pre_join_t<Lhs, JoinType, Rhs>> : public std::true_type {};

// Note: See sqlpp23/core/basic/join_fwd.h for forward declarations.

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, inner_join_t, table_ref_t<Rhs>> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs))};
}

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto inner_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, inner_join_t, table_ref_t<Rhs>> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs))};
}

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto left_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, left_outer_join_t, table_ref_t<Rhs>> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs))};
}

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto right_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, right_outer_join_t, table_ref_t<Rhs>> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs))};
}

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto full_outer_join(Lhs lhs, Rhs rhs)
    -> pre_join_t<table_ref_t<Lhs>, full_outer_join_t, table_ref_t<Rhs>> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs))};
}

template <StaticTable Lhs, DynamicTable Rhs>
  requires(can_be_joined_v<Lhs, Rhs>)
auto cross_join(Lhs lhs, Rhs rhs) -> join_t<table_ref_t<Lhs>,
                                            cross_join_t,
                                            table_ref_t<Rhs>,
                                            unconditional_t> {
  return {make_table_ref(std::move(lhs)), make_table_ref(std::move(rhs)), {}};
}

}  // namespace sqlpp
