#ifndef ARCHER_H
#define ARCHER_H

#include <string>
#include "Character.h"

class Archer : public Character {
public:
    Archer(const string& n);
    int attack() const override;
    int specialAbility() const override;
};

#endif