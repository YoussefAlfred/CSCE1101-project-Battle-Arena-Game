#ifndef MAGE_H
#define MAGE_H

#include "Character.h"

class Mage : public Character {
public:
    explicit Mage(const string& n);

    int  attack()         const override;
    int  specialAbility() const override;
    void move(int dx, int dy) override;
};

#endif