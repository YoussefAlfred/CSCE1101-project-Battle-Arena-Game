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
    int gridX;
    int gridY;
protected:
    int getAttackPower() const;
    int getHealth() const;
    int getGridX() const;
    int getGridY() const;
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
    void move(int x, int y) const;
    void SetPosition(int x, int y) const;
};

#endif
