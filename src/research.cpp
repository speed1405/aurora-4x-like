#include "research.h"
#include <algorithm>

std::string techCategoryToString(TechCategory cat) {
    switch(cat) {
        case TechCategory::PROPULSION: return "Propulsion";
        case TechCategory::WEAPONS: return "Weapons";
        case TechCategory::SENSORS: return "Sensors";
        case TechCategory::SHIELDS: return "Shields";
        case TechCategory::CONSTRUCTION: return "Construction";
        case TechCategory::MINING: return "Mining";
        case TechCategory::POWER: return "Power Generation";
        case TechCategory::BIOLOGY: return "Biology";
        case TechCategory::LOGISTICS: return "Logistics";
        default: return "Unknown";
    }
}

std::string techEraToString(TechEra era) {
    switch(era) {
        case TechEra::PRE_WARP: return "Pre-Warp";
        case TechEra::EARLY_WARP: return "Early Warp";
        case TechEra::INTERSTELLAR: return "Interstellar";
        case TechEra::ADVANCED: return "Advanced";
        case TechEra::FUTURE: return "Future";
        default: return "Unknown";
    }
}

Technology::Technology(const std::string& id, const std::string& nm, TechCategory cat,
                       TechEra er, int cst, const std::vector<std::string>& prereqs,
                       const std::string& desc)
    : techId(id), name(nm), category(cat), era(er), cost(cst),
      prerequisites(prereqs), description(desc), researched(false), progress(0) {}

bool Technology::isAvailable(const std::set<std::string>& researchedTechs) const {
    for (const auto& prereq : prerequisites) {
        if (researchedTechs.find(prereq) == researchedTechs.end()) {
            return false;
        }
    }
    return true;
}

bool Technology::addProgress(int points) {
    if (researched) return true;
    
    progress += points;
    if (progress >= cost) {
        researched = true;
        return true;
    }
    return false;
}

ResearchTree::ResearchTree() {
    initializeTechTree();
}

void ResearchTree::addTech(const std::string& id, const std::string& name,
                           TechCategory cat, TechEra era, int cost,
                           const std::vector<std::string>& prereqs,
                           const std::string& desc) {
    technologies[id] = std::make_shared<Technology>(id, name, cat, era, cost, prereqs, desc);
}

void ResearchTree::initializeTechTree() {
    // Pre-Warp Era
    addTech("basic_mining", "Basic Mining", TechCategory::MINING, TechEra::PRE_WARP,
            100, {}, "Unlocks basic mining facilities");
    addTech("nuclear_power", "Nuclear Power", TechCategory::POWER, TechEra::PRE_WARP,
            150, {}, "Nuclear power generation");
    addTech("ion_drive", "Ion Drive", TechCategory::PROPULSION, TechEra::PRE_WARP,
            200, {}, "Basic ion propulsion for in-system travel");
    addTech("missile_tech", "Missile Technology", TechCategory::WEAPONS, TechEra::PRE_WARP,
            150, {}, "Basic missile weapons");
    addTech("basic_sensors", "Basic Sensors", TechCategory::SENSORS, TechEra::PRE_WARP,
            100, {}, "Short-range detection systems");
    
    // Early Warp Era
    addTech("warp_theory", "Warp Theory", TechCategory::PROPULSION, TechEra::EARLY_WARP,
            500, {"ion_drive", "nuclear_power"}, "Theoretical basis for FTL travel");
    addTech("warp_drive_1", "Warp Drive I", TechCategory::PROPULSION, TechEra::EARLY_WARP,
            1000, {"warp_theory"}, "First generation FTL drive, Warp 1");
    addTech("fusion_power", "Fusion Power", TechCategory::POWER, TechEra::EARLY_WARP,
            600, {"nuclear_power"}, "Fusion reactor technology");
    addTech("laser_weapons", "Laser Weapons", TechCategory::WEAPONS, TechEra::EARLY_WARP,
            700, {"missile_tech"}, "Energy-based weapons");
    addTech("basic_shields", "Basic Shields", TechCategory::SHIELDS, TechEra::EARLY_WARP,
            800, {"fusion_power"}, "Protective energy shields");
    
    // Interstellar Era
    addTech("warp_drive_2", "Warp Drive II", TechCategory::PROPULSION, TechEra::INTERSTELLAR,
            2000, {"warp_drive_1"}, "Improved FTL drive, Warp 2");
    addTech("advanced_mining", "Advanced Mining", TechCategory::MINING, TechEra::INTERSTELLAR,
            1500, {"basic_mining", "fusion_power"}, "Automated mining systems");
    addTech("plasma_weapons", "Plasma Weapons", TechCategory::WEAPONS, TechEra::INTERSTELLAR,
            1800, {"laser_weapons", "fusion_power"}, "Superheated plasma cannons");
    addTech("long_range_sensors", "Long Range Sensors", TechCategory::SENSORS, TechEra::INTERSTELLAR,
            1200, {"basic_sensors"}, "Extended detection range");
    addTech("advanced_shields", "Advanced Shields", TechCategory::SHIELDS, TechEra::INTERSTELLAR,
            2000, {"basic_shields"}, "Improved shield strength");
    addTech("orbital_construction", "Orbital Construction", TechCategory::CONSTRUCTION,
            TechEra::INTERSTELLAR, 1600, {"advanced_mining"}, "Build stations in space");
    
    // Advanced Era
    addTech("warp_drive_3", "Warp Drive III", TechCategory::PROPULSION, TechEra::ADVANCED,
            4000, {"warp_drive_2"}, "Advanced FTL drive, Warp 3");
    addTech("antimatter_power", "Antimatter Power", TechCategory::POWER, TechEra::ADVANCED,
            3500, {"fusion_power"}, "Antimatter reactors");
    addTech("particle_beam", "Particle Beam Weapons", TechCategory::WEAPONS, TechEra::ADVANCED,
            3000, {"plasma_weapons"}, "Particle accelerator weapons");
    addTech("graviton_shields", "Graviton Shields", TechCategory::SHIELDS, TechEra::ADVANCED,
            3500, {"advanced_shields"}, "Gravity-based shields");
    addTech("quantum_sensors", "Quantum Sensors", TechCategory::SENSORS, TechEra::ADVANCED,
            2500, {"long_range_sensors"}, "Quantum entanglement detection");
    
    // Future Era
    addTech("transwarp_drive", "Transwarp Drive", TechCategory::PROPULSION, TechEra::FUTURE,
            8000, {"warp_drive_3", "antimatter_power"}, "Experimental ultra-fast FTL");
    addTech("zero_point_energy", "Zero Point Energy", TechCategory::POWER, TechEra::FUTURE,
            7000, {"antimatter_power"}, "Tap vacuum energy");
    addTech("singularity_weapons", "Singularity Weapons", TechCategory::WEAPONS, TechEra::FUTURE,
            10000, {"particle_beam", "graviton_shields"}, "Micro black hole weapons");
    addTech("phase_shields", "Phase Shields", TechCategory::SHIELDS, TechEra::FUTURE,
            9000, {"graviton_shields"}, "Dimensional phase shifting");
    addTech("terraform_tech", "Terraforming", TechCategory::BIOLOGY, TechEra::FUTURE,
            6000, {"orbital_construction"}, "Transform planetary environments");
}

std::vector<std::shared_ptr<Technology>> ResearchTree::getAvailableTechs() const {
    std::vector<std::shared_ptr<Technology>> available;
    for (const auto& pair : technologies) {
        auto tech = pair.second;
        if (!tech->isResearched() && tech->isAvailable(researched)) {
            available.push_back(tech);
        }
    }
    return available;
}

bool ResearchTree::research(const std::string& techId, int points) {
    auto it = technologies.find(techId);
    if (it == technologies.end()) return false;
    
    auto tech = it->second;
    if (!tech->isAvailable(researched)) return false;
    
    bool completed = tech->addProgress(points);
    if (completed) {
        researched.insert(techId);
    }
    return completed;
}

std::shared_ptr<Technology> ResearchTree::getTech(const std::string& techId) const {
    auto it = technologies.find(techId);
    return (it != technologies.end()) ? it->second : nullptr;
}

bool ResearchTree::isResearched(const std::string& techId) const {
    return researched.find(techId) != researched.end();
}
