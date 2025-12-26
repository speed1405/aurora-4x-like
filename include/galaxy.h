#ifndef GALAXY_H
#define GALAXY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <random>
#include <cstdint>
#include "resources.h"

class Star {
private:
    std::string name;
    std::string starType;

public:
    Star(const std::string& name, std::mt19937& gen, const std::string& type = "");
    
    const std::string& getName() const { return name; }
    const std::string& getStarType() const { return starType; }
};

class Colony;

class Planet {
private:
    std::string name;
    std::string planetType;
    std::map<ResourceType, int> minerals;
    bool colonized;
    std::shared_ptr<Colony> colony;
    
    void generateMinerals(std::mt19937& gen);

public:
    Planet(const std::string& name, std::mt19937& gen, const std::string& type = "");
    
    void colonize(std::shared_ptr<Colony> col);
    
    const std::string& getName() const { return name; }
    const std::string& getPlanetType() const { return planetType; }
    const std::map<ResourceType, int>& getMinerals() const { return minerals; }
    bool isColonized() const { return colonized; }
};

class StarSystem {
private:
    std::string name;
    int x, y, z;
    Star star;
    std::vector<std::shared_ptr<Planet>> planets;
    bool explored;
    
    void generatePlanets(std::mt19937& gen);

public:
    StarSystem(const std::string& name, std::mt19937& gen, int x = 0, int y = 0, int z = 0);
    
    void explore() { explored = true; }
    std::vector<std::shared_ptr<Planet>> getColonizablePlanets() const;
    
    const std::string& getName() const { return name; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getZ() const { return z; }
    const Star& getStar() const { return star; }
    const std::vector<std::shared_ptr<Planet>>& getPlanets() const { return planets; }
    bool isExplored() const { return explored; }
};

class Galaxy {
private:
    std::vector<std::shared_ptr<StarSystem>> systems;
    std::shared_ptr<StarSystem> homeSystem;

    uint32_t seed;
    std::mt19937 gen;
    
    void generateGalaxy(int numSystems);
    std::string generateStarName(int index);

public:
    Galaxy(int numSystems = 20, uint32_t seed = 0);
    
    std::vector<std::shared_ptr<StarSystem>> getExploredSystems() const;
    std::vector<std::shared_ptr<StarSystem>> getUnexploredSystems() const;

    uint32_t getSeed() const { return seed; }
    std::shared_ptr<StarSystem> findSystemByName(const std::string& name) const;
    
    const std::vector<std::shared_ptr<StarSystem>>& getSystems() const { return systems; }
    std::shared_ptr<StarSystem> getHomeSystem() const { return homeSystem; }
};

#endif // GALAXY_H
