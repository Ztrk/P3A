# CMAKE_BUILD_TYPE: Release, Debug, Relwithdebinfo, ...

cmake_minimum_required(VERSION 3.0)

project(probabilistic-port-planning-algorithm)

include_directories(src)

add_executable(p3a src/main.cpp src/local_search.cpp)

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(p3a PRIVATE "-Wall" "-Wextra")
endif()


# Tests setup
add_subdirectory(lib/googletest-release-1.10.0)

add_executable(p3a_test test/local_search_test.cpp src/local_search.cpp)
target_link_libraries(p3a_test gtest_main)

enable_testing()
add_test(NAME p3a_test COMMAND p3a_test)
