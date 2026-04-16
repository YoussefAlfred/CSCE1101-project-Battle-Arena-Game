#include "Character.h"
#include <string>
#include <iostream>

int Character::characterCount = 0;

int Character::getAttackPower() const {
    return attackPower;
}
int Character::getHealth() const {
    return health;
}
Character::Character(const string& n, int HP, int aP){
    name = n;
    health = HP;
    attackPower = aP;
    characterCount++;
}
Character::~Character() {
    characterCount--;
    cout << "Character " << name << " has been destroyed." << endl;
}
string Character::getName() const {
    return name;
}
void Character::takeDamage(int damage) {
    health = health - damage;
    if (health < 0) {
        health = 0;
    }
}
bool Character::isAlive() const {
    return health > 0;
}
int Character::getCharacterCount() {
    return characterCount;
}
int Character::getCurrentHealth() const {
    return getHealth();
}
int Character::getGridX() const {
    return gridX;
}
int Character::getGridY() const {
    return gridY;
}
void Character::move(int x, int y) const{
    gridX += x;
    gridY += y;
}
void Character::SetPosition(int x, int y) const {
    gridX = x;
    gridY = y;
}
