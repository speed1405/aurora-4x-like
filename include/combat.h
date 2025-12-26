#ifndef COMBAT_H
#define COMBAT_H

#include <string>
#include <vector>
#include <memory>

enum class ShipClass {
    SCOUT,
    FIGHTER,
    CORVETTE,
    FRIGATE,
    DESTROYER,
    CRUISER,
    BATTLESHIP,
    CARRIER
};

std::string shipClassToString(ShipClass sc);

class Weapon {
private:
    std::string name;
    int damage;
    double accuracy;
    int range;

public:
    Weapon(const std::string& name, int dmg, double acc, int rng);
    
    int fire() const;
    const std::string& getName() const { return name; }
};

class Ship {
private:
    std::string name;
    ShipClass shipClass;
    int maxHull;
    int hull;
    int maxShields;
    int shields;
    std::vector<Weapon> weapons;
    bool destroyed;

public:
    Ship(const std::string& name, ShipClass sc, int hull, int shields,
         const std::vector<Weapon>& wpns = {});
    
    void takeDamage(int damage);
    int fireAt();
    bool isOperational() const { return !destroyed && hull > 0; }
    
    const std::string& getName() const { return name; }
    ShipClass getShipClass() const { return shipClass; }
    int getHull() const { return hull; }
    int getMaxHull() const { return maxHull; }
    int getShields() const { return shields; }
    int getMaxShields() const { return maxShields; }
};

class StarSystem;

class Fleet {
private:
    std::string name;
    std::string owner;
    std::vector<std::shared_ptr<Ship>> ships;
    std::shared_ptr<StarSystem> location;

public:
    Fleet(const std::string& name, const std::string& owner);
    
    void addShip(std::shared_ptr<Ship> ship);
    void removeDestroyed();
    int getCombatStrength() const;
    bool isDefeated() const;
    
    const std::string& getName() const { return name; }
    const std::vector<std::shared_ptr<Ship>>& getShips() const { return ships; }
    std::vector<std::shared_ptr<Ship>>& getShips() { return ships; }
    void setLocation(std::shared_ptr<StarSystem> sys) { location = sys; }
    std::shared_ptr<StarSystem> getLocation() const { return location; }
};

class Combat {
private:
    std::shared_ptr<Fleet> attacker;
    std::shared_ptr<Fleet> defender;
    std::vector<std::string> combatLog;
    int round;

public:
    Combat(std::shared_ptr<Fleet> atk, std::shared_ptr<Fleet> def);
    
    void resolveRound();
    std::shared_ptr<Fleet> resolve(int maxRounds = 10);
    const std::vector<std::string>& getLog() const { return combatLog; }
};

#endif // COMBAT_H
