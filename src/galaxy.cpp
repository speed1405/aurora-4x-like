#include "galaxy.h"
#include "empire.h"
#include <random>
#include <algorithm>

Star::Star(const std::string& nm, const std::string& type) : name(nm) {
    static const std::vector<std::string> types = {
        "Red Dwarf", "Yellow Dwarf", "Blue Giant", "Red Giant", "White Dwarf"
    };
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    if (type.empty()) {
        std::uniform_int_distribution<> dis(0, types.size() - 1);
        starType = types[dis(gen)];
    } else {
        starType = type;
    }
}

Planet::Planet(const std::string& nm, const std::string& type)
    : name(nm), colonized(false) {
    static const std::vector<std::string> types = {
        "Terrestrial", "Gas Giant", "Ice", "Desert", "Ocean", "Volcanic"
    };
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    if (type.empty()) {
        std::uniform_int_distribution<> dis(0, types.size() - 1);
        planetType = types[dis(gen)];
    } else {
        planetType = type;
    }
    
    generateMinerals();
}

void Planet::generateMinerals() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> chance(0.0, 1.0);
    std::uniform_int_distribution<> amount(1000, 100000);
    
    std::vector<ResourceType> mineralTypes = {
        ResourceType::DURANIUM, ResourceType::NEUTRONIUM, ResourceType::CORUNDIUM,
        ResourceType::TRITANIUM, ResourceType::BORONIDE, ResourceType::MERCASSIUM,
        ResourceType::VENDARITE, ResourceType::SORIUM, ResourceType::URIDIUM,
        ResourceType::GALLICITE
    };
    
    for (const auto& mineral : mineralTypes) {
        if (chance(gen) > 0.3) {  // 70% chance to have each mineral
            minerals[mineral] = amount(gen);
        }
    }
}

void Planet::colonize(std::shared_ptr<Colony> col) {
    colonized = true;
    colony = col;
}

StarSystem::StarSystem(const std::string& nm, int posX, int posY, int posZ)
    : name(nm), x(posX), y(posY), z(posZ), star(nm + " Primary"), explored(false) {
    generatePlanets();
}

void StarSystem::generatePlanets() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> numPlanets(2, 10);
    
    int count = numPlanets(gen);
    for (int i = 0; i < count; ++i) {
        std::string planetName = name + " " + char('A' + i);
        planets.push_back(std::make_shared<Planet>(planetName));
    }
}

std::vector<std::shared_ptr<Planet>> StarSystem::getColonizablePlanets() const {
    std::vector<std::shared_ptr<Planet>> colonizable;
    for (const auto& planet : planets) {
        if ((planet->getPlanetType() == "Terrestrial" || planet->getPlanetType() == "Ocean") 
            && !planet->isColonized()) {
            colonizable.push_back(planet);
        }
    }
    return colonizable;
}

Galaxy::Galaxy(int numSystems) {
    generateGalaxy(numSystems);
}

void Galaxy::generateGalaxy(int numSystems) {
    // Create home system
    homeSystem = std::make_shared<StarSystem>("Sol", 0, 0, 0);
    homeSystem->explore();
    systems.push_back(homeSystem);
    
    // Generate other systems
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> xDist(-50, 50);
    std::uniform_int_distribution<> yDist(-50, 50);
    std::uniform_int_distribution<> zDist(-20, 20);
    
    for (int i = 0; i < numSystems - 1; ++i) {
        std::string name = generateStarName(i);
        int x = xDist(gen);
        int y = yDist(gen);
        int z = zDist(gen);
        systems.push_back(std::make_shared<StarSystem>(name, x, y, z));
    }
}

std::string Galaxy::generateStarName(int index) {
    static const std::vector<std::string> prefixes = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta",
        "Theta", "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron"
    };
    static const std::vector<std::string> suffixes = {
        "Centauri", "Draconis", "Eridani", "Cygni", "Leonis", "Aquarii",
        "Cassiopeiae", "Orionis", "Pegasi", "Andromedae"
    };
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> prefixDist(0, prefixes.size() - 1);
    std::uniform_int_distribution<> suffixDist(0, suffixes.size() - 1);
    
    return prefixes[prefixDist(gen)] + " " + suffixes[suffixDist(gen)];
}

std::vector<std::shared_ptr<StarSystem>> Galaxy::getExploredSystems() const {
    std::vector<std::shared_ptr<StarSystem>> explored;
    for (const auto& sys : systems) {
        if (sys->isExplored()) {
            explored.push_back(sys);
        }
    }
    return explored;
}

std::vector<std::shared_ptr<StarSystem>> Galaxy::getUnexploredSystems() const {
    std::vector<std::shared_ptr<StarSystem>> unexplored;
    for (const auto& sys : systems) {
        if (!sys->isExplored()) {
            unexplored.push_back(sys);
        }
    }
    return unexplored;
}
