cmake_minimum_required(VERSION 3.31)
project(sudoku)

set(CMAKE_CXX_STANDARD 20)

# --- The normal Executable ---

add_executable(sudoku src/main.cpp
        src/Sudoku.cpp
)

target_include_directories(sudoku PRIVATE include)

# --- The Benchmark Executable ---

find_package(benchmark REQUIRED)

add_executable(sudoku_bench
        benchmark/benchmark_main.cpp
        src/Sudoku.cpp
)

# Link the benchmark library
target_link_libraries(sudoku_bench PRIVATE benchmark::benchmark)
target_include_directories(sudoku_bench PRIVATE include)

# Ensure Release build for benchmarks (Debug is too slow for accurate timing)
# We override the global Debug setting just for the bench target
set_target_properties(sudoku_bench PROPERTIES
        CXX_FLAGS_RELEASE "-O3 -DNDEBUG"
)

# Set default build type to Release for the bench target if not specified
if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
endif()
