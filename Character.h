#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
using namespace std;
class Character {
private:
    string name;
    int health;
    int attackPower;
    static int characterCount;
protected:
    int getAttackPower() const;
    int getHealth() const;
public:
    Character(const string& n, int HP, int aP);
    string getName() const;
    virtual int attack() const = 0;
    virtual int specialAbility() const = 0;
    void takeDamage(int damage);
    bool isAlive() const;
    static int getCharacterCount();
    virtual ~Character();
    int getCurrentHealth() const;
};

#endif
