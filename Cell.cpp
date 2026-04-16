#include "Cell.h"

Cell::Cell() : row(0), col(0), occupied(false), occupant(nullptr) {}

Cell::Cell(int row, int col) : row(row), col(col), occupied(false), occupant(nullptr) {}

bool Cell::isOccupied() const {
    return occupied;
}

void Cell::setOccupant(Character* character) {
    occupant = character;
    occupied = (character != nullptr);
}

void Cell::clearOccupant() {
    occupant = nullptr;
    occupied = false;
}

Character* Cell::getOccupant() const {
    return occupant;
}

int Cell::getRow() const {
    return row;
}

int Cell::getCol() const {
    return col;
}
