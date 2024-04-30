cmake_minimum_required(VERSION 3.20)
project(tensile)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(tensilelib)

add_executable(tensile main.cpp)
target_link_libraries(tensile tensilelib duckdb pqxx libsqlite3.a)

add_subdirectory(duckdb)
#add_subdirectory(libpqxx)
#add_subdirectory(sqlite-amalgamation)

