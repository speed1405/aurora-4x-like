#include "combat.h"
#include <algorithm>
#include <random>
#include <sstream>

std::string shipClassToString(ShipClass sc) {
    switch(sc) {
        case ShipClass::SCOUT: return "Scout";
        case ShipClass::FIGHTER: return "Fighter";
        case ShipClass::CORVETTE: return "Corvette";
        case ShipClass::FRIGATE: return "Frigate";
        case ShipClass::DESTROYER: return "Destroyer";
        case ShipClass::CRUISER: return "Cruiser";
        case ShipClass::BATTLESHIP: return "Battleship";
        case ShipClass::CARRIER: return "Carrier";
        default: return "Unknown";
    }
}

Weapon::Weapon(const std::string& nm, int dmg, double acc, int rng)
    : name(nm), damage(dmg), accuracy(acc), range(rng) {}

int Weapon::fire() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    if (dis(gen) < accuracy) {
        return damage;
    }
    return 0;
}

Ship::Ship(const std::string& nm, ShipClass sc, int hll, int shlds,
           const std::vector<Weapon>& wpns)
    : name(nm), shipClass(sc), maxHull(hll), hull(hll),
      maxShields(shlds), shields(shlds), weapons(wpns), destroyed(false) {}

void Ship::takeDamage(int damage) {
    // Shields absorb first
    if (shields > 0) {
        int absorbed = std::min(shields, damage);
        shields -= absorbed;
        damage -= absorbed;
    }
    
    // Remaining damage to hull
    if (damage > 0) {
        hull -= damage;
        if (hull <= 0) {
            hull = 0;
            destroyed = true;
        }
    }
}

int Ship::fireAt() {
    int totalDamage = 0;
    for (const auto& weapon : weapons) {
        totalDamage += weapon.fire();
    }
    return totalDamage;
}

Fleet::Fleet(const std::string& nm, const std::string& own)
    : name(nm), owner(own) {}

void Fleet::addShip(std::shared_ptr<Ship> ship) {
    ships.push_back(ship);
}

void Fleet::removeDestroyed() {
    ships.erase(
        std::remove_if(ships.begin(), ships.end(),
                      [](const std::shared_ptr<Ship>& ship) { return !ship->isOperational(); }),
        ships.end()
    );
}

int Fleet::getCombatStrength() const {
    int strength = 0;
    for (const auto& ship : ships) {
        if (ship->isOperational()) {
            strength += ship->getHull() + ship->getShields();
        }
    }
    return strength;
}

bool Fleet::isDefeated() const {
    return std::all_of(ships.begin(), ships.end(),
                      [](const std::shared_ptr<Ship>& ship) { return !ship->isOperational(); });
}

Combat::Combat(std::shared_ptr<Fleet> atk, std::shared_ptr<Fleet> def)
    : attacker(atk), defender(def), round(0) {}

void Combat::resolveRound() {
    round++;
    std::stringstream ss;
    ss << "=== Combat Round " << round << " ===";
    combatLog.push_back(ss.str());
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // Attacker fires
    for (auto& ship : attacker->getShips()) {
        if (ship->isOperational() && !defender->getShips().empty()) {
            std::vector<std::shared_ptr<Ship>> operationalTargets;
            for (auto& target : defender->getShips()) {
                if (target->isOperational()) {
                    operationalTargets.push_back(target);
                }
            }
            
            if (!operationalTargets.empty()) {
                std::uniform_int_distribution<> dis(0, operationalTargets.size() - 1);
                auto target = operationalTargets[dis(gen)];
                
                int damage = ship->fireAt();
                if (damage > 0) {
                    target->takeDamage(damage);
                    std::stringstream log;
                    log << ship->getName() << " hits " << target->getName() << " for " << damage << " damage";
                    combatLog.push_back(log.str());
                    
                    if (!target->isOperational()) {
                        combatLog.push_back(target->getName() + " destroyed!");
                    }
                }
            }
        }
    }
    
    // Defender fires back
    for (auto& ship : defender->getShips()) {
        if (ship->isOperational() && !attacker->getShips().empty()) {
            std::vector<std::shared_ptr<Ship>> operationalTargets;
            for (auto& target : attacker->getShips()) {
                if (target->isOperational()) {
                    operationalTargets.push_back(target);
                }
            }
            
            if (!operationalTargets.empty()) {
                std::uniform_int_distribution<> dis(0, operationalTargets.size() - 1);
                auto target = operationalTargets[dis(gen)];
                
                int damage = ship->fireAt();
                if (damage > 0) {
                    target->takeDamage(damage);
                    std::stringstream log;
                    log << ship->getName() << " hits " << target->getName() << " for " << damage << " damage";
                    combatLog.push_back(log.str());
                    
                    if (!target->isOperational()) {
                        combatLog.push_back(target->getName() + " destroyed!");
                    }
                }
            }
        }
    }
    
    // Remove destroyed ships
    attacker->removeDestroyed();
    defender->removeDestroyed();
}

std::shared_ptr<Fleet> Combat::resolve(int maxRounds) {
    while (round < maxRounds) {
        resolveRound();
        
        if (attacker->isDefeated()) {
            combatLog.push_back(defender->getName() + " wins!");
            return defender;
        } else if (defender->isDefeated()) {
            combatLog.push_back(attacker->getName() + " wins!");
            return attacker;
        }
    }
    
    // If max rounds reached, check who has more strength
    int attackerStrength = attacker->getCombatStrength();
    int defenderStrength = defender->getCombatStrength();
    
    if (attackerStrength > defenderStrength) {
        combatLog.push_back(attacker->getName() + " wins by attrition!");
        return attacker;
    } else {
        combatLog.push_back(defender->getName() + " wins by attrition!");
        return defender;
    }
}
