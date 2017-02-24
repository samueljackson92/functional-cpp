cmake_minimum_required (VERSION 3.0)
project (Functional)

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_SOURCE_DIR}/Catch/include)

add_library(hana INTERFACE)
target_include_directories(hana INTERFACE ${CMAKE_SOURCE_DIR}/hana/include)

add_library(ranges INTERFACE)
target_include_directories(ranges INTERFACE ${CMAKE_SOURCE_DIR}/range-v3/include)

add_executable(all_tests composition.hpp test.cpp)
target_link_libraries(all_tests Catch hana ranges)
