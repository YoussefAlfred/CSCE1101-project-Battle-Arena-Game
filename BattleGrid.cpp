#include "BattleGrid.h"
#include "Character.h"
#include <cstdlib> // for abs()

BattleGrid::BattleGrid() {
    reset();
}

void BattleGrid::reset() {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            grid[r][c] = Cell(r, c);
            blocked[r][c] = false;
        }
    }
    initializeObstacles();
}

void BattleGrid::initializeObstacles() {
    // Symmetric ruins: enough cover to change movement, but both corners keep routes.
    const int obstacles[][2] = {
        {1, 3}, {2, 3}, {3, 1}, {3, 5},
        {4, 2}, {4, 6}, {5, 4}, {6, 4}
    };

    for (const auto& pos : obstacles) {
        blocked[pos[0]][pos[1]] = true;
    }
}

bool BattleGrid::placeCharacter(Character* character, int row, int col) {
    if (!isValidMove(row, col)) return false;
    if (grid[row][col].isOccupied()) return false;

    grid[row][col].setOccupant(character);
    character->SetPosition(row, col);  
    return true;
}

bool BattleGrid::moveCharacter(Character* character, int newRow, int newCol) {
    if (!isValidMove(newRow, newCol)) return false;
    if (grid[newRow][newCol].isOccupied()) return false;

    // Clear old cell
    int oldRow = character->getGridX();
    int oldCol = character->getGridY();
    grid[oldRow][oldCol].clearOccupant();

    // Place in new cell
    grid[newRow][newCol].setOccupant(character);
    character->SetPosition(newRow, newCol);
    return true;
}

bool BattleGrid::isAdjacent(int row1, int col1, int row2, int col2) const {
    int dr = std::abs(row1 - row2);
    int dc = std::abs(col1 - col2);
    // Adjacent = exactly 1 step horizontally or vertically (no diagonals)
    return (dr + dc) == 1;
}

bool BattleGrid::isValidMove(int row, int col) const {
    return isInside(row, col) && !isBlocked(row, col);
}

bool BattleGrid::isBlocked(int row, int col) const {
    return isInside(row, col) && blocked[row][col];
}

bool BattleGrid::isInside(int row, int col) const {
    return row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE;
}

Cell* BattleGrid::getCell(int row, int col) {
    if (!isInside(row, col)) return nullptr;
    return &grid[row][col];
}
