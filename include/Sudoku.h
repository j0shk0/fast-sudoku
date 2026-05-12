#ifndef SUDOKU_SUDOKU_H
#define SUDOKU_SUDOKU_H

#include <bitset>
#include <string>

class Sudoku {
  [[nodiscard]] bool checkRow(const size_t &row, const size_t &col);

  [[nodiscard]] bool checkColumn(const size_t &row, const size_t &col) const;

  [[nodiscard]] bool checkBox(const size_t &row, const size_t &col) const;

  int board_[81]{};

  bool rowState_[9]{};

  bool columnState_[81]{};

  bool boxState_[81]{};

 public:
  Sudoku(int solution[81]);

  Sudoku();

  void generate();

  void printBoard() const;

  std::string getBoardString() const;

  bool check();
};

#endif  // SUDOKU_SUDOKU_H
