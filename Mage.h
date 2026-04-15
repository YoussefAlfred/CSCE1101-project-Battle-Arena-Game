#ifndef MAGE_H
#define MAGE_H

#include <string>
#include "Character.h"

class Mage : public Character {
public:
    Mage(const string& n);
    int attack() const override;
    int specialAbility() const override;
};

#endif