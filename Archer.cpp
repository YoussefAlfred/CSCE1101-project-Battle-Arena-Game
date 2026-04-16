#include "Archer.h"
#include <iostream>

// Spec: 150 HP, 15 ATK
Archer::Archer(const string& n) : Character(n, 150, 15) {}

int Archer::attack() const {
    return getAttackPower(); // 15
}

int Archer::specialAbility() const {
    // Double Shot: 2× ATK
    return getAttackPower() * 2;
}

void Archer::move(int dx, int dy) {
    SetPosition(getGridX() + dx, getGridY() + dy);
}