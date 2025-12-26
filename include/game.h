#ifndef GAME_H
#define GAME_H

#include <memory>
#include <string>
#include <vector>
#include "empire.h"
#include "galaxy.h"
#include "combat.h"

class Game {
private:
    std::shared_ptr<Empire> empire;
    std::shared_ptr<Galaxy> galaxy;
    bool running;
    
    void setupGame();
    std::shared_ptr<Fleet> createStartingFleet();

public:
    Game(const std::string& empireName = "Earth Empire");
    
    std::string advanceTurn();
    std::string exploreSystem(const std::string& systemName);
    std::string startResearch(const std::string& techId);
    std::vector<std::shared_ptr<Technology>> getAvailableResearch();
    std::string buildShip(ShipClass shipClass, const std::string& fleetName);
    std::string simulateCombat(const std::string& fleet1Name, const std::string& fleet2Name);
    
    std::shared_ptr<Empire> getEmpire() { return empire; }
    std::shared_ptr<Galaxy> getGalaxy() { return galaxy; }
};

#endif // GAME_H
