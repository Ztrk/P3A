# CMAKE_BUILD_TYPE: Release, Debug, Relwithdebinfo, ...

cmake_minimum_required(VERSION 3.0)

project(probabilistic-port-planning-algorithm)

find_package(MPI REQUIRED)
include_directories(src)

add_executable(p3a src/main.cpp src/local_search.cpp src/evaluator.cpp src/mpi_evaluator.cpp 
    src/instance_generator.cpp src/greedy.cpp src/ils.cpp)

target_include_directories(p3a PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_compile_options(p3a PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(p3a PRIVATE ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(p3a PRIVATE "-Wall" "-Wextra")
endif()


# Tests setup
add_subdirectory(lib/googletest-release-1.10.0)

add_executable(p3a_test test/local_search_test.cpp src/local_search.cpp)
target_link_libraries(p3a_test PRIVATE gtest_main)

enable_testing()
add_test(NAME p3a_test COMMAND p3a_test)
