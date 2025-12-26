#include "empire.h"
#include "galaxy.h"

Colony::Colony(const std::string& nm, std::shared_ptr<Planet> plt)
    : name(nm), planet(plt), population(10), infrastructure(1), mines(0), factories(0) {}

void Colony::grow() {
    double growthRate = 0.01 * infrastructure;
    population += static_cast<int>(population * growthRate);
}

Empire::Empire(const std::string& nm)
    : name(nm), turn(0), totalPopulation(100), militaryStrength(0) {}

std::string Empire::advanceTurn() {
    turn++;
    
    // Produce resources
    resources.produce();
    
    // Research progress
    if (!currentResearch.empty()) {
        int researchPoints = resources.get(ResourceType::RESEARCH_POINTS);
        if (researchPoints > 0) {
            int pointsToUse = std::min(researchPoints, 10);
            bool completed = research.research(currentResearch, pointsToUse);
            resources.consume(ResourceType::RESEARCH_POINTS, pointsToUse);
            
            if (completed) {
                auto tech = research.getTech(currentResearch);
                currentResearch = "";
                return "Turn " + std::to_string(turn) + " completed. Research completed: " + tech->getName() + "!";
            }
        }
    }
    
    // Colony growth
    for (auto& colony : colonies) {
        colony->grow();
    }
    
    return "Turn " + std::to_string(turn) + " completed";
}

bool Empire::setResearch(const std::string& techId) {
    auto tech = research.getTech(techId);
    if (tech && !tech->isResearched() && tech->isAvailable(research.getResearchedCount() ? std::set<std::string>() : std::set<std::string>())) {
        // Need to get the actual researched set from the tree
        auto available = research.getAvailableTechs();
        for (const auto& availTech : available) {
            if (availTech->getId() == techId) {
                currentResearch = techId;
                return true;
            }
        }
    }
    return false;
}

void Empire::addColony(std::shared_ptr<Colony> colony) {
    colonies.push_back(colony);
}

void Empire::addFleet(std::shared_ptr<Fleet> fleet) {
    fleets.push_back(fleet);
}
