//
// Created by j0shk0 on 07/04/2026.
//

#include <benchmark/benchmark.h>
#include "Sudoku.h"

namespace {

    // Benchmark the generate function
    void BM_SudokuGenerate(benchmark::State& state) {
        for (auto _ : state) {
            Sudoku sudoku;
            sudoku.generate();
            // Prevent the compiler from optimizing away the call
            benchmark::DoNotOptimize(sudoku);
        }
    }

}

    // Register the benchmark
    BENCHMARK(BM_SudokuGenerate)->Unit(benchmark::kMillisecond);

    // Run the main
    BENCHMARK_MAIN();
