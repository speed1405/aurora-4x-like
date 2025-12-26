#ifndef RESEARCH_H
#define RESEARCH_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>

enum class TechCategory {
    PROPULSION,
    WEAPONS,
    SENSORS,
    SHIELDS,
    CONSTRUCTION,
    MINING,
    POWER,
    BIOLOGY,
    LOGISTICS
};

enum class TechEra {
    PRE_WARP,
    EARLY_WARP,
    INTERSTELLAR,
    ADVANCED,
    FUTURE
};

std::string techCategoryToString(TechCategory cat);
std::string techEraToString(TechEra era);

class Technology {
private:
    std::string techId;
    std::string name;
    TechCategory category;
    TechEra era;
    int cost;
    std::vector<std::string> prerequisites;
    std::string description;
    bool researched;
    int progress;

public:
    Technology(const std::string& id, const std::string& name, TechCategory cat,
               TechEra era, int cost, const std::vector<std::string>& prereqs = {},
               const std::string& desc = "");
    
    bool isAvailable(const std::set<std::string>& researchedTechs) const;
    bool addProgress(int points);
    
    const std::string& getId() const { return techId; }
    const std::string& getName() const { return name; }
    TechCategory getCategory() const { return category; }
    TechEra getEra() const { return era; }
    int getCost() const { return cost; }
    int getProgress() const { return progress; }
    bool isResearched() const { return researched; }
    const std::vector<std::string>& getPrerequisites() const { return prerequisites; }
};

class ResearchTree {
private:
    std::map<std::string, std::shared_ptr<Technology>> technologies;
    std::set<std::string> researched;
    
    void initializeTechTree();
    void addTech(const std::string& id, const std::string& name, TechCategory cat,
                 TechEra era, int cost, const std::vector<std::string>& prereqs,
                 const std::string& desc);

public:
    ResearchTree();
    
    std::vector<std::shared_ptr<Technology>> getAvailableTechs() const;
    bool research(const std::string& techId, int points);
    std::shared_ptr<Technology> getTech(const std::string& techId) const;
    bool isResearched(const std::string& techId) const;
    int getResearchedCount() const { return static_cast<int>(researched.size()); }
};

#endif // RESEARCH_H
