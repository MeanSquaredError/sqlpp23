# Copyright (c) 2013 - 2021, Roland Bock
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


function(create_test name)
    set(target sqlpp23_sqlite3_usage_${name})
    add_executable(${target} ${name}.cpp)
    sqlpp_slite3_test_add_target_link_libraries(${target})
    add_test(NAME ${target} COMMAND ${target})
endfunction()

create_test(exceptions)
create_test(exists)
create_test(operator)
add_subdirectory(aggregate_function)
add_subdirectory(statement)

set(test_files
    Attach.cpp
    AutoIncrement.cpp
    Blob.cpp
    Connection.cpp
    ConnectionPool.cpp
    DateTime.cpp
    DynamicSelect.cpp
    Execute.cpp
    FloatingPoint.cpp
    InsertOnConflict.cpp
    Integral.cpp
    Returning.cpp
    Sample.cpp
    Select.cpp
    Transaction.cpp
    Union.cpp
    With.cpp
)

create_test_sourcelist(test_sources test_main.cpp ${test_files})

add_executable(sqlpp23_sqlite3_tests ${test_sources})
target_link_libraries(sqlpp23_sqlite3_tests PRIVATE sqlpp23_testing sqlpp23_sqlite3_testing)
if (BUILD_SQLCIPHER_CONNECTOR)
    target_link_libraries(sqlpp23_sqlite3_tests PRIVATE sqlpp23::sqlcipher)
else()
    target_link_libraries(sqlpp23_sqlite3_tests PRIVATE sqlpp23::sqlite3)
endif()

foreach(test_file IN LISTS test_files)
    get_filename_component(test ${test_file} NAME_WLE)
    add_test(NAME sqlpp23.sqlite3.usage.${test}
        COMMAND sqlpp23_sqlite3_tests ${test}
    )
endforeach()


