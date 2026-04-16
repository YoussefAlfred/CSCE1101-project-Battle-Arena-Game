#pragma once
#include "Cell.h"

class Character; // forward declaration

class BattleGrid {
public:
    BattleGrid();

    bool placeCharacter(Character* character, int row, int col);
    bool moveCharacter(Character* character, int newRow, int newCol);

    bool isAdjacent(int row1, int col1, int row2, int col2) const;
    bool isValidMove(int row, int col) const;

    Cell* getCell(int row, int col);

    static const int GRID_SIZE = 8;

private:
    Cell grid[GRID_SIZE][GRID_SIZE];
};
