# CMakeLists files in this project can
cmake_minimum_required (VERSION 2.8.11)

project( distr_log_db )
set(CMAKE_BUILD_TYPE Debug)

add_library (
    distr_log_db
    app/src/b_plus.cpp
    app/src/leaf.cpp
    app/src/tree.cpp
    )

target_compile_options( distr_log_db PUBLIC -std=c++11 )

target_include_directories (distr_log_db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app/include)

add_executable(
    distr_log_db_test
    test/test.cpp
)

target_link_libraries( distr_log_db_test distr_log_db )
