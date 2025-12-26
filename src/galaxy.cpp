#include "galaxy.h"
#include "empire.h"
#include <algorithm>
#include <cctype>
#include <random>

Star::Star(const std::string& nm, std::mt19937& gen, const std::string& type) : name(nm) {
    static const std::vector<std::string> types = {
        "Red Dwarf", "Yellow Dwarf", "Blue Giant", "Red Giant", "White Dwarf"
    };
    
    if (type.empty()) {
        std::uniform_int_distribution<std::size_t> dis(0, types.size() - 1);
        starType = types[dis(gen)];
    } else {
        starType = type;
    }
}

Planet::Planet(const std::string& nm, std::mt19937& gen, const std::string& type)
    : name(nm), colonized(false) {
    static const std::vector<std::string> types = {
        "Terrestrial", "Gas Giant", "Ice", "Desert", "Ocean", "Volcanic"
    };
    
    if (type.empty()) {
        std::uniform_int_distribution<std::size_t> dis(0, types.size() - 1);
        planetType = types[dis(gen)];
    } else {
        planetType = type;
    }
    
    generateMinerals(gen);
}

void Planet::generateMinerals(std::mt19937& gen) {
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

StarSystem::StarSystem(const std::string& nm, std::mt19937& gen, int posX, int posY, int posZ)
    : name(nm), x(posX), y(posY), z(posZ), star(nm + " Primary", gen), explored(false) {
    generatePlanets(gen);
}

void StarSystem::generatePlanets(std::mt19937& gen) {
    std::uniform_int_distribution<> numPlanets(2, 10);
    
    int count = numPlanets(gen);
    for (int i = 0; i < count; ++i) {
        std::string planetName = name + " " + char('A' + i);
        planets.push_back(std::make_shared<Planet>(planetName, gen));
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

Galaxy::Galaxy(int numSystems, uint32_t seed)
    : seed(seed ? seed : std::random_device{}()), gen(this->seed) {
    generateGalaxy(numSystems);
}

void Galaxy::generateGalaxy(int numSystems) {
    // Create home system
    homeSystem = std::make_shared<StarSystem>("Sol", gen, 0, 0, 0);
    homeSystem->explore();
    systems.push_back(homeSystem);
    
    // Generate other systems
    std::uniform_int_distribution<> xDist(-50, 50);
    std::uniform_int_distribution<> yDist(-50, 50);
    std::uniform_int_distribution<> zDist(-20, 20);
    
    for (int i = 0; i < numSystems - 1; ++i) {
        std::string name = generateStarName(i);
        int x = xDist(gen);
        int y = yDist(gen);
        int z = zDist(gen);
        systems.push_back(std::make_shared<StarSystem>(name, gen, x, y, z));
    }
}

std::string Galaxy::generateStarName(int index) {
    (void)index;
    static const std::vector<std::string> prefixes = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta",
        "Theta", "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron"
    };
    static const std::vector<std::string> suffixes = {
        "Centauri", "Draconis", "Eridani", "Cygni", "Leonis", "Aquarii",
        "Cassiopeiae", "Orionis", "Pegasi", "Andromedae"
    };
    
    std::uniform_int_distribution<std::size_t> prefixDist(0, prefixes.size() - 1);
    std::uniform_int_distribution<std::size_t> suffixDist(0, suffixes.size() - 1);
    
    return prefixes[prefixDist(gen)] + " " + suffixes[suffixDist(gen)];
}

std::shared_ptr<StarSystem> Galaxy::findSystemByName(const std::string& name) const {
    auto toLowerChar = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::string want = name;
    std::transform(want.begin(), want.end(), want.begin(), toLowerChar);
    for (const auto& sys : systems) {
        if (!sys) continue;
        std::string cur = sys->getName();
        std::transform(cur.begin(), cur.end(), cur.begin(), toLowerChar);
        if (cur == want) return sys;
    }
    return nullptr;
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
