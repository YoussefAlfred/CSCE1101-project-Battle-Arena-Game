#include "Character.h"
#include <string>
#include <iostream>

int Character::characterCount = 0;

Character::Character(const string& n, int HP, int aP)
    : name(n), health(HP), maxHealth(HP), attackPower(aP), gridX(0), gridY(0)
{
    characterCount++;
}

Character::~Character() {
    characterCount--;
}

int    Character::getAttackPower() const { return attackPower; }
int    Character::getHealth()      const { return health; }
string Character::getName()        const { return name; }

void Character::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;
}

bool Character::isAlive()          const { return health > 0; }
int  Character::getCurrentHealth() const { return health; }
int  Character::getMaxHealth()     const { return maxHealth; }   // ← added

int  Character::getGridX() const { return gridX; }
int  Character::getGridY() const { return gridY; }

void Character::SetPosition(int x, int y) {
    gridX = x;
    gridY = y;
}

int Character::getCharacterCount() { return characterCount; }