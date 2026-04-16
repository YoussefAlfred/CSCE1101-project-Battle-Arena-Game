#include "Mage.h"
#include <string>
#include <iostream>
Mage::Mage(const string& n) : Character(n, 150, 30) {
}
int Mage::attack() const {
    return getAttackPower();
}
int Mage::specialAbility() const {
    cout << getName() << " uses Arcane Storm!" << endl;
    return getAttackPower() * 3;
}