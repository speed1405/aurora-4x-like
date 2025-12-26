#ifndef EMPIRE_H
#define EMPIRE_H

#include <string>
#include <vector>
#include <memory>
#include "resources.h"
#include "research.h"

class Planet;

class Colony {
private:
    std::string name;
    std::shared_ptr<Planet> planet;
    int population;
    int infrastructure;
    int mines;
    int factories;

public:
    Colony(const std::string& name, std::shared_ptr<Planet> planet);
    
    void grow();
    void buildMine() { mines++; }
    void buildFactory() { factories++; }

    void setPopulationForLoad(int p) { population = p; }
    void setMinesForLoad(int v) { mines = v; }
    void setFactoriesForLoad(int v) { factories = v; }
    
    const std::string& getName() const { return name; }
    std::shared_ptr<Planet> getPlanet() const { return planet; }
    int getPopulation() const { return population; }
    int getMines() const { return mines; }
    int getFactories() const { return factories; }
};

class Fleet;

class Empire {
private:
    std::string name;
    ResourceStorage resources;
    ResearchTree research;
    std::vector<std::shared_ptr<Colony>> colonies;
    std::vector<std::shared_ptr<Fleet>> fleets;
    int turn;
    std::string currentResearch;
    int totalPopulation;
    int militaryStrength;

public:
    Empire(const std::string& name = "Earth Empire");
    
    std::string advanceTurn();
    bool setResearch(const std::string& techId);
    void setTurnForLoad(int t) { turn = t; }
    void setCurrentResearchForLoad(const std::string& techId) { currentResearch = techId; }
    void addColony(std::shared_ptr<Colony> colony);
    void addFleet(std::shared_ptr<Fleet> fleet);
    
    const std::string& getName() const { return name; }
    int getTurn() const { return turn; }
    ResourceStorage& getResources() { return resources; }
    const ResourceStorage& getResources() const { return resources; }
    ResearchTree& getResearch() { return research; }
    const ResearchTree& getResearch() const { return research; }
    const std::vector<std::shared_ptr<Colony>>& getColonies() const { return colonies; }
    const std::vector<std::shared_ptr<Fleet>>& getFleets() const { return fleets; }
    const std::string& getCurrentResearch() const { return currentResearch; }
};

#endif // EMPIRE_H
