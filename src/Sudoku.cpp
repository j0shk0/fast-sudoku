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
    std::set<int> backtracked_values{};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            // Determine possible candidates for the current cell.
            int possible_values[]{1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::vector<int> candidates;
            candidates.reserve(9);
            std::ranges::copy_if(possible_values, std::back_inserter(candidates),
                                 [this, &i, &j, &backtracked_values](const int v) {
                                     board_[(i * 9) + j] = v;
                                     const auto result = checkRow(i, j) && checkColumn(i, j) &&
                                                         checkBox(i, j) && !backtracked_values.contains(v);
                                     return result;
                                 });
            board_[(i * 9) + j] = 0;
            // Add backtracking logic.
            if (candidates.empty()) {
                if (j != 0) {
                    backtracked_values.insert(board_[(i * 9) + --j]);
                    board_[(i * 9) + j--] = 0;
                } else {
                    j = 8;
                    backtracked_values.insert(board_[(--i * 9) + j]);
                    board_[(i * 9) + j--] = 0;
                }
                if (backtracked_values.size() == 9) {
                    std::ranges::fill(board_, 0);
                    std::ranges::fill(rowState_, false);
                    i = -1;
                    j = 0;
                    backtracked_values.clear();
                    break;
                }
                continue;
            }
            thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_int_distribution<std::size_t> dist(0, candidates.size() - 1);
            const auto r = dist(rng);
            auto it = std::begin(candidates);
            std::advance(it, r);
            board_[(i * 9) + j] = *it;
            rowState_[board_[(i * 9) + j] - 1] = true;
        }
    }
}

bool Sudoku::checkRow(const size_t &row, const size_t &col) {
    if (col == 0) {
        std::ranges::fill(rowState_, false);
        return true;
    }
    const bool existing_value = rowState_[board_[(row * 9) + col] - 1];
    rowState_[board_[(row * 9) + col] - 1] = true;
    const auto unique_vals = std::accumulate(std::begin(rowState_), std::end(rowState_), 0);
    rowState_[board_[(row * 9) + col] - 1] = existing_value ? existing_value : false;
    return unique_vals == col + 1;
}

bool Sudoku::checkColumn(const size_t &row, const size_t &col) const {
    if (row == 0)
        return true;
    bool col_check[9]{};
    for (size_t i = 0; i < row + 1; i++) {
        if (board_[(i * 9) + col] == 0) continue;
        col_check[board_[(i * 9) + col] - 1] = true;
    }
    return std::accumulate(std::begin(col_check), std::end(col_check), 0) == row + 1;
}

bool Sudoku::checkBox(const size_t &row, const size_t &col) const {
    const size_t box_row = row / 3;
    const size_t box_col = col / 3;

    bool box_check[9]{};
    for (size_t i = box_row * 3; i < (box_row * 3) + 3; i++) {
        for (size_t j = box_col * 3; j < (box_col * 3) + 3; j++) {
            if (board_[(i * 9) + j] == 0) continue;
            box_check[board_[(i * 9) + j] - 1] = true;
        }
    }
    const size_t box_vals = (3 * ((row % 3) + 1)) - (3 - ((col % 3) + 1));
    return std::accumulate(std::begin(box_check), std::end(box_check),
                           size_t{0}) == box_vals;
}

void Sudoku::printBoard() const {
    for (auto row = 0; row < 9; row++) {
        for (auto col = 0; col < 9; col++) {
            std::cout << board_[(row * 9) + col] << " ";
        }
        std::cout << std::endl;
    }
}
