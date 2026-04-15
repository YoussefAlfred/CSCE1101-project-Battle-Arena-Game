#ifndef WARRIOR_H
#define WARRIOR_H

#include <string>
#include "Character.h"

class Warrior : public Character {
public:
    Warrior(const string& n);
    int attack() const override;
    int specialAbility() const override;
};

#endif