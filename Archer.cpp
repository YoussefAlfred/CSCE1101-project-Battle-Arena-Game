#include "Archer.h"
#include <string>
#include <iostream>
Archer::Archer(const string& n) : Character(n, 200, 50) {
}
int Archer::attack() const {
    return getAttackPower();
}
int Archer::specialAbility() const {
    cout << getName() << " uses Double Shot!" << endl;
    return getAttackPower() * 2;
}
