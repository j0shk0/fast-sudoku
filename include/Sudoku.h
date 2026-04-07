//
// Created by j0shk0 on 01/04/2026.
//

#ifndef SUDOKU_SUDOKU_H
#define SUDOKU_SUDOKU_H

#include <string>
#include <bitset>
#include <array>

class Sudoku {

    [[nodiscard]] bool checkRow(const size_t &row, const size_t &col);

    [[nodiscard]] bool checkColumn(const size_t &row, const size_t &col) const;

    [[nodiscard]] bool checkBox(const size_t &row, const size_t &col) const;

public:

    int board_[81]{};

    bool rowState_[9]{};

    static bool check(const std::string &potentialSolution);

    void generate();

    void printBoard() const;
};

#endif //SUDOKU_SUDOKU_H
