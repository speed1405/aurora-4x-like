#ifndef GAME_H
#define GAME_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "empire.h"
#include "galaxy.h"
#include "combat.h"

class Game {
private:
    std::shared_ptr<Empire> empire;
    std::shared_ptr<Galaxy> galaxy;
    std::vector<std::shared_ptr<Empire>> hostileEmpires;
    std::map<std::string, bool> hostileContacted;
    std::map<std::string, bool> hostileAtWar;
    bool running;
    
    void setupGame();
    std::shared_ptr<Fleet> createStartingFleet();

public:
    Game(const std::string& empireName = "Earth Empire", uint32_t galaxySeed = 0);
    
    std::string advanceTurn();
    std::string exploreSystem(const std::string& systemName);
    std::string startResearch(const std::string& techId);
    std::vector<std::shared_ptr<Technology>> getAvailableResearch();
    std::string buildShip(ShipClass shipClass, const std::string& fleetName);
    std::string simulateCombat(const std::string& fleet1Name, const std::string& fleet2Name);

    std::string quickSave(const std::string& path = "savegame.txt") const;
    std::string quickLoad(const std::string& path = "savegame.txt");
    
    std::shared_ptr<Empire> getEmpire() { return empire; }
    std::shared_ptr<Galaxy> getGalaxy() { return galaxy; }
    const std::vector<std::shared_ptr<Empire>>& getHostileEmpires() const { return hostileEmpires; }

    bool isHostileContacted(const std::string& hostileName) const;
    bool isHostileAtWar(const std::string& hostileName) const;
};

#endif // GAME_H
