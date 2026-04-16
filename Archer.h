#ifndef ARCHER_H
#define ARCHER_H

#include "Character.h"

class Archer : public Character {
public:
    explicit Archer(const string& n);

    int  attack()         const override;
    int  specialAbility() const override;
    void move(int dx, int dy) override;
};

#endif