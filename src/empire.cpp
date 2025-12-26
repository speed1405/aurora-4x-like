#include "empire.h"
#include "galaxy.h"
#include <algorithm>

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
        auto tech = research.getTech(currentResearch);
        const int availableRP = resources.get(ResourceType::RESEARCH_POINTS);
        const int maxSpendPerTurn = 10;

        if (tech && availableRP > 0) {
            const int remaining = std::max(0, tech->getCost() - tech->getProgress());
            const int pointsToUse = std::min({availableRP, maxSpendPerTurn, remaining});

            if (pointsToUse > 0) {
                const int beforeProgress = tech->getProgress();
                const bool completed = research.research(currentResearch, pointsToUse);
                const int afterProgress = tech->getProgress();
                const int spent = std::max(0, afterProgress - beforeProgress);

                if (spent > 0) {
                    resources.consume(ResourceType::RESEARCH_POINTS, spent);
                } else {
                    // Research could not advance (typically unmet prerequisites); do not consume RP.
                    return "Turn " + std::to_string(turn) + " completed. Research blocked: prerequisites not met for " +
                           tech->getName() + ".";
                }

                if (completed) {
                    const std::string techName = tech->getName();
                    currentResearch = "";
                    return "Turn " + std::to_string(turn) + " completed. Spent " +
                           std::to_string(spent) + " RP. Research completed: " + techName + "!";
                }

                return "Turn " + std::to_string(turn) + " completed. Spent " +
                       std::to_string(spent) + " RP on " + tech->getName() +
                       " (" + std::to_string(tech->getProgress()) + "/" + std::to_string(tech->getCost()) + ")";
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
    if (!tech || tech->isResearched()) {
        return false;
    }

    // Only allow selecting technologies that are currently available given prerequisites.
    const auto available = research.getAvailableTechs();
    for (const auto& availTech : available) {
        if (availTech && availTech->getId() == techId) {
            currentResearch = techId;
            return true;
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
