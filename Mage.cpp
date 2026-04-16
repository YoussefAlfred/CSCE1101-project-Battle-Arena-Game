#include "Mage.h"
#include <iostream>

// Spec: 100 HP, 20 ATK
Mage::Mage(const string& n) : Character(n, 100, 20) {}

int Mage::attack() const {
    return getAttackPower(); // 20
}

int Mage::specialAbility() const {
    // Arcane Storm: 3× ATK
    return getAttackPower() * 3;
}

void Mage::move(int dx, int dy) {
    SetPosition(getGridX() + dx, getGridY() + dy);
}