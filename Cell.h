#pragma once

class Character; // forward declaration

class Cell {
public:
    Cell();
    Cell(int row, int col);

    bool isOccupied() const;

    void setOccupant(Character* character);
    void clearOccupant();
    Character* getOccupant() const;

    int getRow() const;
    int getCol() const;

private:
    int row;
    int col;
    bool occupied;
    Character* occupant;
};
