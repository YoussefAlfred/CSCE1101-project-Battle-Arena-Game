#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
using namespace std;

class Character {
private:
    string name;
    int health;
    int maxHealth;      
    int attackPower;
    static int characterCount;
    int gridX = 0;
    int gridY = 0;

protected:
    int getAttackPower() const;
    int getHealth()      const;

public:
    Character(const string& n, int HP, int aP);
    virtual ~Character();

    string getName() const;

    // Pure virtuals — every subclass must implement these
    virtual int  attack()         const = 0;
    virtual int  specialAbility() const = 0;
    virtual void move(int dx, int dy)   = 0;

    void takeDamage(int damage);
    void heal(int amount);
    bool isAlive()           const;
    int  getCurrentHealth()  const;
    int  getMaxHealth()      const;

    int  getGridX() const;
    int  getGridY() const;
    void SetPosition(int x, int y);

    static int getCharacterCount();
};

#endif