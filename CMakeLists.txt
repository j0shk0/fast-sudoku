cmake_minimum_required(VERSION 3.31)
project(sudoku)

set(CMAKE_CXX_STANDARD 20)

# --- The Benchmark ---
find_package(benchmark REQUIRED)
find_package(Boost REQUIRED)

add_executable(sudoku_bench
        benchmark/benchmark_main.cpp
        src/Sudoku.cpp
)

# --- The Server ---
add_executable(server
        src/server.cpp
        src/Sudoku.cpp
)

# Link the benchmark library and include dir
target_link_libraries(sudoku_bench PRIVATE benchmark::benchmark)
target_include_directories(sudoku_bench PRIVATE include)

# Link boost and include dir
target_link_libraries(server PRIVATE Boost::boost)
target_include_directories(server PRIVATE include)

# Ensure Release build for benchmarks (Debug is too slow for accurate timing)
# We override the global Debug setting just for the bench target
set_target_properties(sudoku_bench PROPERTIES
        CXX_FLAGS_RELEASE "-O3 -DNDEBUG"
)

# Set default build type to Release for the bench target if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
