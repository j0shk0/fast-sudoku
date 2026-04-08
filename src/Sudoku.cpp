//
// Created by j0shk0 on 01/04/2026.
//

#include "Sudoku.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <vector>

void Sudoku::generate() {
    bool backtracked_values[9]{};
    size_t backtracked_values_count = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            // Determine possible candidates for the current cell.
            int candidates[9];
            size_t candidates_count = 0;
            for (int v = 1; v <= 9; v++) {
                board_[(i * 9) + j] = v;
                if (checkRow(i, j) && checkColumn(i, j) && checkBox(i, j)
                    && !backtracked_values[v-1]) {
                    candidates[candidates_count++] = v;
                    }
            }
            board_[(i * 9) + j] = 0;
            // Add backtracking logic.
            if (candidates_count == 0) {
                if (j != 0) {
                    backtracked_values[board_[(i * 9) + --j] - 1] = true;
                    backtracked_values_count++;
                    board_[(i * 9) + j--] = 0;
                } else {
                    j = 8;
                    backtracked_values[board_[(--i * 9) + j] - 1] = true;
                    backtracked_values_count++;
                    board_[(i * 9) + j--] = 0;
                }
                if (backtracked_values_count == 9) {
                    std::ranges::fill(board_, 0);
                    std::ranges::fill(rowState_, false);
                    std::ranges::fill(columnState_, false);
                    std::ranges::fill(boxState_, false);
                    std::ranges::fill(backtracked_values, false);
                    backtracked_values_count = 0;
                    i = -1;
                    j = 0;
                    break;
                }
                continue;
            }
            thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_int_distribution<std::size_t> dist(0, candidates_count - 1);
            const auto r = dist(rng);
            auto* it = std::begin(candidates);
            std::advance(it, r);
            board_[(i * 9) + j] = *it;

            rowState_[board_[(i * 9) + j] - 1] = true;
            columnState_[(j * 9) + (board_[(i * 9) + j] - 1)] = true;
            const size_t box_row = i / 3;
            const size_t box_col = j / 3;
            boxState_[(((3 * box_row) + box_col) * 9) + (board_[(i * 9) + j] - 1)] = true;
        }
    }
}

bool Sudoku::checkRow(const size_t &row, const size_t &col) {
    if (col == 0) {
        std::ranges::fill(rowState_, false);
        return true;
    }
    const bool existing_value = rowState_[board_[(row * 9) + col] - 1];
    return !existing_value;
}

bool Sudoku::checkColumn(const size_t &row, const size_t &col) const {
    if (row == 0) {
        return true;
    }
    const bool existing_value = columnState_[(col * 9) + (board_[(row * 9) + col] - 1)];
    return !existing_value;
}

bool Sudoku::checkBox(const size_t &row, const size_t &col) const {
    const size_t box_row = row / 3;
    const size_t box_col = col / 3;

    const bool existing_value = boxState_[(((3 * box_row) + box_col) * 9) + (board_[(row * 9) + col] - 1)];
    return !existing_value;
}

void Sudoku::printBoard() const {
    for (auto row = 0; row < 9; row++) {
        for (auto col = 0; col < 9; col++) {
            std::cout << board_[(row * 9) + col] << " ";
        }
        std::cout << std::endl;
    }
}
