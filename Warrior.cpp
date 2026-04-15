#include "Warrior.h"
#include <string>
#include <iostream>
Warrior::Warrior(const string& n) : Character(n, 300, 80) {
}
int Warrior::attack() const {
    return getAttackPower();
}
int Warrior::specialAbility() const {
    cout << getName() << " uses Power Strike!" << endl;
    return getAttackPower() * 1.5;
}