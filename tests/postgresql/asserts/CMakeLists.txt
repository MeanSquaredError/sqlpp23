# Copyright (c) 2025, Roland Bock
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

function(create_test_for_setup name)
    set(target sqlpp23_postgresql_asserts_setup_${name})
    add_executable(${target} ${name}.cpp)
    target_link_libraries(${target} PRIVATE sqlpp23::postgresql sqlpp23_testing sqlpp23_postgresql_testing)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(test_assert name pattern)
    create_test_for_setup(${name})
    set(test sqlpp23_postgresql_asserts_${name})
    set(target sqlpp23_postgresql_asserts_${name})
    add_executable(${target} EXCLUDE_FROM_ALL ${name}.cpp)
    target_link_libraries(${target} PRIVATE sqlpp23::postgresql sqlpp23_testing sqlpp23_postgresql_testing)
    target_compile_definitions(${target} PRIVATE SQLPP_CHECK_STATIC_ASSERT)
    add_test(NAME ${test}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target}
    )
    set_property(TEST ${test} PROPERTY PASS_REGULAR_EXPRESSION ${pattern})
endfunction()

# Compiling these is required to fail (testing some static_assert)

# Connection
test_assert(bad_execute "cannot execute statements with parameters directly")
test_assert(bad_prepare "at least one selected column requires a table")

test_assert(incompatible_execute "No support for using incompatible expression")
test_assert(incompatible_prepare "No support for using incompatible expression")
