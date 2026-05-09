#pragma once
#include "Cell.h"

class Character; // forward declaration

enum class SpellType { NONE, HEAL, SLOW };

class BattleGrid {
public:
    BattleGrid();

    void reset();
    bool placeCharacter(Character* character, int row, int col);
    bool moveCharacter(Character* character, int newRow, int newCol);

    bool isAdjacent(int row1, int col1, int row2, int col2) const;
    bool isValidMove(int row, int col) const;
    bool isBlocked(int row, int col) const;

    Cell* getCell(int row, int col);

    // Spell rune methods
    SpellType getSpell(int row, int col) const;
    SpellType consumeSpell(int row, int col); // returns type then clears it

    static const int GRID_SIZE = 8;

private:
    bool isInside(int row, int col) const;
    void initializeObstacles();
    void initializeSpells();

    Cell      grid[GRID_SIZE][GRID_SIZE];
    bool      blocked[GRID_SIZE][GRID_SIZE];
    SpellType spells[GRID_SIZE][GRID_SIZE];
};
