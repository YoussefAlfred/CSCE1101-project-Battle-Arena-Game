#ifndef WARRIOR_H
#define WARRIOR_H

#include "Character.h"

class Warrior : public Character {
public:
    explicit Warrior(const string& n);

    int  attack()         const override;
    int  specialAbility() const override;
    void move(int dx, int dy) override;
};

#endif