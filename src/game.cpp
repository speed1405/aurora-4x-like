#include "game.h"
#include <algorithm>
#include <cctype>

Game::Game(const std::string& empireName)
    : empire(std::make_shared<Empire>(empireName)),
      galaxy(std::make_shared<Galaxy>(20)),
      running(false) {
    setupGame();
}

void Game::setupGame() {
    // Colonize home planet (Earth)
    auto homePlanets = galaxy->getHomeSystem()->getPlanets();
    if (homePlanets.size() >= 3) {
        auto homePlanet = homePlanets[2];  // 3rd planet
        auto earthColony = std::make_shared<Colony>("Earth", homePlanet);
        homePlanet->colonize(earthColony);
        empire->addColony(earthColony);
    }
    
    // Create starting fleet
    auto startingFleet = createStartingFleet();
    empire->addFleet(startingFleet);
}

std::shared_ptr<Fleet> Game::createStartingFleet() {
    auto fleet = std::make_shared<Fleet>("Home Defense Fleet", empire->getName());
    
    // Add basic ships
    Weapon laser("Laser Cannon", 10, 0.7, 5);
    
    auto scout = std::make_shared<Ship>("Scout-1", ShipClass::SCOUT, 50, 20,
                                       std::vector<Weapon>{laser});
    auto corvette = std::make_shared<Ship>("Corvette-1", ShipClass::CORVETTE, 100, 50,
                                          std::vector<Weapon>{laser, laser});
    
    fleet->addShip(scout);
    fleet->addShip(corvette);
    fleet->setLocation(galaxy->getHomeSystem());
    
    return fleet;
}

std::string Game::advanceTurn() {
    return empire->advanceTurn();
}

std::string Game::exploreSystem(const std::string& systemName) {
    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };

    for (auto& system : galaxy->getSystems()) {
        std::string sysNameLower = system->getName();
        std::string searchNameLower = systemName;
        std::transform(sysNameLower.begin(), sysNameLower.end(), sysNameLower.begin(), toLowerChar);
        std::transform(searchNameLower.begin(), searchNameLower.end(), searchNameLower.begin(), toLowerChar);
        
        if (sysNameLower == searchNameLower) {
            const bool wasExplored = system->isExplored();
            system->explore();

            if (!wasExplored) {
                const int reward = 10 + static_cast<int>(system->getPlanets().size()) * 2;
                empire->getResources().add(ResourceType::RESEARCH_POINTS, reward);
                return "Explored " + system->getName() + "! Found " +
                       std::to_string(system->getPlanets().size()) + " planets. Gained " +
                       std::to_string(reward) + " research points.";
            }

            return "System already explored: " + system->getName();
        }
    }
    return "System not found";
}

std::string Game::startResearch(const std::string& techId) {
    if (empire->setResearch(techId)) {
        auto tech = empire->getResearch().getTech(techId);
        if (tech) {
            return "Now researching: " + tech->getName();
        }
    }
    return "Cannot research that technology";
}

std::vector<std::shared_ptr<Technology>> Game::getAvailableResearch() {
    return empire->getResearch().getAvailableTechs();
}

std::string Game::buildShip(ShipClass shipClass, const std::string& fleetName) {
    // Find fleet
    std::shared_ptr<Fleet> targetFleet;
    std::string fleetNameLower = fleetName;
    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    std::transform(fleetNameLower.begin(), fleetNameLower.end(), fleetNameLower.begin(), toLowerChar);
    
    for (auto& fleet : empire->getFleets()) {
        std::string fname = fleet->getName();
        std::transform(fname.begin(), fname.end(), fname.begin(), toLowerChar);
        if (fname == fleetNameLower) {
            targetFleet = fleet;
            break;
        }
    }
    
    if (!targetFleet) {
        return "Fleet not found";
    }
    
    // Simple ship creation
    std::string shipName = shipClassToString(shipClass) + "-" + 
                          std::to_string(targetFleet->getShips().size() + 1);
    
    Weapon laser("Laser", 10, 0.7, 5);
    std::shared_ptr<Ship> ship;
    
    if (shipClass == ShipClass::SCOUT) {
        ship = std::make_shared<Ship>(shipName, shipClass, 50, 20,
                                     std::vector<Weapon>{laser});
    } else if (shipClass == ShipClass::CORVETTE) {
        ship = std::make_shared<Ship>(shipName, shipClass, 100, 50,
                                     std::vector<Weapon>{laser, laser});
    } else if (shipClass == ShipClass::FRIGATE) {
        Weapon betterLaser("Heavy Laser", 15, 0.75, 6);
        ship = std::make_shared<Ship>(shipName, shipClass, 200, 100,
                                     std::vector<Weapon>{betterLaser, betterLaser, betterLaser});
    } else {
        return "Unknown ship class";
    }
    
    targetFleet->addShip(ship);
    return "Built " + shipName + " and added to " + targetFleet->getName();
}

std::string Game::simulateCombat(const std::string& fleet1Name, const std::string& fleet2Name) {
    std::shared_ptr<Fleet> fleet1, fleet2;

    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    
    std::string f1Lower = fleet1Name, f2Lower = fleet2Name;
    std::transform(f1Lower.begin(), f1Lower.end(), f1Lower.begin(), toLowerChar);
    std::transform(f2Lower.begin(), f2Lower.end(), f2Lower.begin(), toLowerChar);
    
    for (auto& fleet : empire->getFleets()) {
        std::string fname = fleet->getName();
        std::transform(fname.begin(), fname.end(), fname.begin(), toLowerChar);
        if (fname == f1Lower) fleet1 = fleet;
        if (fname == f2Lower) fleet2 = fleet;
    }
    
    if (!fleet1 || !fleet2) {
        return "One or both fleets not found";
    }
    
    Combat combat(fleet1, fleet2);
    combat.resolve();
    
    std::string result;
    for (const auto& log : combat.getLog()) {
        result += log + "\n";
    }
    return result;
}
