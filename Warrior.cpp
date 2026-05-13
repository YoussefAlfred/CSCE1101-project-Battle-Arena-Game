#include "Warrior.h"
#include <iostream>

// Spec: 200 HP, 20 ATK
Warrior::Warrior(const string& n) : Character(n, 200, 20) {}

int Warrior::attack() const {
    return getAttackPower(); // 20
}

int Warrior::specialAbility() const {
    // Power Strike: 1.5× ATK (returns int)
    return static_cast<int>(getAttackPower() * 1.5);
}

void Warrior::move(int dx, int dy) {
    SetPosition(getGridX() + dx, getGridY() + dy);
}
