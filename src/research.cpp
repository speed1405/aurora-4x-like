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

void Technology::setProgressForLoad(int p) {
        progress = std::max(0, p);
        if (progress > cost) progress = cost;
}

void Technology::setResearchedForLoad(bool v) {
        researched = v;
        if (researched) progress = cost;
}

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
    addTech("industrial_engineering", "Industrial Engineering", TechCategory::CONSTRUCTION, TechEra::PRE_WARP,
            120, {"basic_mining"}, "Improves basic industry and construction methods");
    addTech("sensor_fusion", "Sensor Fusion", TechCategory::SENSORS, TechEra::PRE_WARP,
            150, {"basic_sensors"}, "Combine multiple short-range sensors into a unified picture");
    addTech("reactor_safety", "Reactor Safety", TechCategory::POWER, TechEra::PRE_WARP,
            110, {"nuclear_power"}, "Improves reactor reliability and safety protocols");
    addTech("improved_mining", "Improved Mining", TechCategory::MINING, TechEra::PRE_WARP,
            180, {"basic_mining"}, "More efficient extraction and refining");
    addTech("basic_logistics", "Basic Logistics", TechCategory::LOGISTICS, TechEra::PRE_WARP,
            140, {}, "Improves storage, distribution, and supply planning");
    addTech("medical_infrastructure", "Medical Infrastructure", TechCategory::BIOLOGY, TechEra::PRE_WARP,
            130, {}, "Improves population health and long-term survival");
    addTech("nuclear_power", "Nuclear Power", TechCategory::POWER, TechEra::PRE_WARP,
            150, {}, "Nuclear power generation");
    addTech("ion_drive", "Ion Drive", TechCategory::PROPULSION, TechEra::PRE_WARP,
            200, {}, "Basic ion propulsion for in-system travel");
    addTech("missile_tech", "Missile Technology", TechCategory::WEAPONS, TechEra::PRE_WARP,
            150, {}, "Basic missile weapons");
    addTech("basic_sensors", "Basic Sensors", TechCategory::SENSORS, TechEra::PRE_WARP,
            100, {}, "Short-range detection systems");
    addTech("gauss_theory", "Gauss Theory", TechCategory::WEAPONS, TechEra::PRE_WARP,
            160, {"missile_tech"}, "Foundations of electromagnetic acceleration weapons");
    addTech("hull_plating", "Hull Plating", TechCategory::CONSTRUCTION, TechEra::PRE_WARP,
            170, {"industrial_engineering"}, "Improved structural integrity and armor plating");
    
    // Early Warp Era
    addTech("warp_theory", "Warp Theory", TechCategory::PROPULSION, TechEra::EARLY_WARP,
            500, {"ion_drive", "nuclear_power"}, "Theoretical basis for FTL travel");
    addTech("warp_navigation", "Warp Navigation", TechCategory::SENSORS, TechEra::EARLY_WARP,
            650, {"warp_theory", "basic_sensors"}, "Navigation and detection techniques for early FTL travel");
    addTech("warp_drive_1", "Warp Drive I", TechCategory::PROPULSION, TechEra::EARLY_WARP,
            1000, {"warp_theory"}, "First generation FTL drive, Warp 1");
    addTech("fusion_power", "Fusion Power", TechCategory::POWER, TechEra::EARLY_WARP,
            600, {"nuclear_power"}, "Fusion reactor technology");
    addTech("laser_weapons", "Laser Weapons", TechCategory::WEAPONS, TechEra::EARLY_WARP,
            700, {"missile_tech"}, "Energy-based weapons");
    addTech("point_defense", "Point Defense", TechCategory::WEAPONS, TechEra::EARLY_WARP,
            600, {"laser_weapons", "basic_sensors"}, "Close-in defensive weapon systems against missiles and fighters");
    addTech("gauss_weapons", "Gauss Weapons", TechCategory::WEAPONS, TechEra::EARLY_WARP,
            750, {"gauss_theory", "fusion_power"}, "Electromagnetic projectile weapons");
    addTech("logistics_network", "Logistics Network", TechCategory::LOGISTICS, TechEra::EARLY_WARP,
            650, {"basic_logistics", "fusion_power"}, "Interplanetary supply coordination and distribution");
    addTech("reinforced_hulls", "Reinforced Hulls", TechCategory::CONSTRUCTION, TechEra::EARLY_WARP,
            700, {"hull_plating", "fusion_power"}, "Improved structural reinforcement for larger ships");
    addTech("basic_shields", "Basic Shields", TechCategory::SHIELDS, TechEra::EARLY_WARP,
            800, {"fusion_power"}, "Protective energy shields");
    addTech("shield_harmonics", "Shield Harmonics", TechCategory::SHIELDS, TechEra::EARLY_WARP,
            900, {"basic_shields"}, "Improves shield stability and field coherence");
    
    // Interstellar Era
    addTech("warp_drive_2", "Warp Drive II", TechCategory::PROPULSION, TechEra::INTERSTELLAR,
            2000, {"warp_drive_1"}, "Improved FTL drive, Warp 2");
    addTech("warp_efficiency", "Warp Field Efficiency", TechCategory::PROPULSION, TechEra::INTERSTELLAR,
            1600, {"warp_drive_1", "fusion_power"}, "Improves fuel usage and reliability of warp fields");
    addTech("advanced_mining", "Advanced Mining", TechCategory::MINING, TechEra::INTERSTELLAR,
            1500, {"basic_mining", "fusion_power"}, "Automated mining systems");
    addTech("asteroid_mining", "Asteroid Mining", TechCategory::MINING, TechEra::INTERSTELLAR,
            1400, {"advanced_mining", "orbital_construction"}, "Extract minerals from asteroids and small bodies");
    addTech("plasma_weapons", "Plasma Weapons", TechCategory::WEAPONS, TechEra::INTERSTELLAR,
            1800, {"laser_weapons", "fusion_power"}, "Superheated plasma cannons");
    addTech("railgun_weapons", "Railgun Weapons", TechCategory::WEAPONS, TechEra::INTERSTELLAR,
            1600, {"gauss_weapons", "fusion_power"}, "High-velocity kinetic weapons with improved penetrative power");
    addTech("missile_guidance", "Advanced Missile Guidance", TechCategory::WEAPONS, TechEra::INTERSTELLAR,
            1400, {"missile_tech", "long_range_sensors"}, "Improves hit chance at long range");
    addTech("long_range_sensors", "Long Range Sensors", TechCategory::SENSORS, TechEra::INTERSTELLAR,
            1200, {"basic_sensors"}, "Extended detection range");
    addTech("sensor_arrays", "Sensor Arrays", TechCategory::SENSORS, TechEra::INTERSTELLAR,
            1700, {"long_range_sensors"}, "Large-scale sensor installations with improved sensitivity");
    addTech("ecm_suite", "ECM Suite", TechCategory::SENSORS, TechEra::INTERSTELLAR,
            1600, {"long_range_sensors", "laser_weapons"}, "Electronic countermeasures and signal disruption");
    addTech("advanced_shields", "Advanced Shields", TechCategory::SHIELDS, TechEra::INTERSTELLAR,
            2000, {"basic_shields"}, "Improved shield strength");
    addTech("shield_regeneration", "Shield Regeneration", TechCategory::SHIELDS, TechEra::INTERSTELLAR,
            1800, {"advanced_shields", "fusion_power"}, "Faster shield recovery and capacitor cycling");
    addTech("orbital_construction", "Orbital Construction", TechCategory::CONSTRUCTION,
            TechEra::INTERSTELLAR, 1600, {"advanced_mining"}, "Build stations in space");
    addTech("shipyard_construction", "Shipyard Construction", TechCategory::CONSTRUCTION, TechEra::INTERSTELLAR,
            1500, {"orbital_construction", "reinforced_hulls"}, "Enables larger-scale ship construction in orbit");
    addTech("colony_hydroponics", "Colony Hydroponics", TechCategory::BIOLOGY, TechEra::INTERSTELLAR,
            1200, {"logistics_network", "fusion_power"}, "Improves colony self-sufficiency and growth");
    
    // Advanced Era
    addTech("warp_drive_3", "Warp Drive III", TechCategory::PROPULSION, TechEra::ADVANCED,
            4000, {"warp_drive_2"}, "Advanced FTL drive, Warp 3");
    addTech("warp_stabilization", "Warp Field Stabilization", TechCategory::PROPULSION, TechEra::ADVANCED,
            3600, {"warp_drive_3", "warp_efficiency"}, "Stabilizes high-energy warp fields for safer travel");
    addTech("antimatter_power", "Antimatter Power", TechCategory::POWER, TechEra::ADVANCED,
            3500, {"fusion_power"}, "Antimatter reactors");
    addTech("power_distribution_mk2", "Power Distribution Mk II", TechCategory::POWER, TechEra::ADVANCED,
            2400, {"antimatter_power"}, "Improves power routing and reduces waste heat");
    addTech("particle_beam", "Particle Beam Weapons", TechCategory::WEAPONS, TechEra::ADVANCED,
            3000, {"plasma_weapons"}, "Particle accelerator weapons");
    addTech("antimatter_torpedoes", "Antimatter Torpedoes", TechCategory::WEAPONS, TechEra::ADVANCED,
            3500, {"antimatter_power", "plasma_weapons"}, "High-yield warheads requiring antimatter containment");
    addTech("graviton_shields", "Graviton Shields", TechCategory::SHIELDS, TechEra::ADVANCED,
            3500, {"advanced_shields"}, "Gravity-based shields");
    addTech("shield_overchargers", "Shield Overchargers", TechCategory::SHIELDS, TechEra::ADVANCED,
            3200, {"graviton_shields", "power_distribution_mk2"}, "Temporarily boost shield capacity under fire");
    addTech("quantum_sensors", "Quantum Sensors", TechCategory::SENSORS, TechEra::ADVANCED,
            2500, {"long_range_sensors"}, "Quantum entanglement detection");
    addTech("advanced_ecm", "Advanced ECM", TechCategory::SENSORS, TechEra::ADVANCED,
            2800, {"ecm_suite", "quantum_sensors"}, "Adaptive electronic warfare and decoys");
    addTech("advanced_shipyards", "Advanced Shipyards", TechCategory::CONSTRUCTION, TechEra::ADVANCED,
            3200, {"shipyard_construction", "antimatter_power"}, "High-throughput orbital ship construction");
    addTech("nanomaterials", "Nanomaterials", TechCategory::CONSTRUCTION, TechEra::ADVANCED,
            3000, {"reinforced_hulls"}, "Strong, lightweight structural materials");
    addTech("fleet_coordination_ai", "Fleet Coordination AI", TechCategory::LOGISTICS, TechEra::ADVANCED,
            2500, {"logistics_network", "quantum_sensors"}, "Improves fleet command, control, and response time");
    addTech("genetic_adaptation", "Genetic Adaptation", TechCategory::BIOLOGY, TechEra::ADVANCED,
            2600, {"colony_hydroponics"}, "Adapt organisms for harsher environments and long-duration travel");
    addTech("deep_core_mining", "Deep Core Mining", TechCategory::MINING, TechEra::ADVANCED,
            2800, {"asteroid_mining", "antimatter_power"}, "Extreme-environment extraction and processing");
    
    // Future Era
    addTech("transwarp_drive", "Transwarp Drive", TechCategory::PROPULSION, TechEra::FUTURE,
            8000, {"warp_drive_3", "antimatter_power"}, "Experimental ultra-fast FTL");
    addTech("zero_point_energy", "Zero Point Energy", TechCategory::POWER, TechEra::FUTURE,
            7000, {"antimatter_power"}, "Tap vacuum energy");
    addTech("quantum_singularity_containment", "Quantum Singularity Containment", TechCategory::POWER, TechEra::FUTURE,
            9000, {"zero_point_energy"}, "Contain extreme energies for advanced reactors and weapons");
    addTech("singularity_weapons", "Singularity Weapons", TechCategory::WEAPONS, TechEra::FUTURE,
            10000, {"particle_beam", "graviton_shields"}, "Micro black hole weapons");
    addTech("temporal_weapons", "Temporal Weapons", TechCategory::WEAPONS, TechEra::FUTURE,
            12000, {"singularity_weapons", "transwarp_drive"}, "Exotic weapons that distort local spacetime");
    addTech("phase_shields", "Phase Shields", TechCategory::SHIELDS, TechEra::FUTURE,
            9000, {"graviton_shields"}, "Dimensional phase shifting");
    addTech("void_shields", "Void Shields", TechCategory::SHIELDS, TechEra::FUTURE,
            9500, {"phase_shields", "zero_point_energy"}, "Shields that partially decouple from normal space");
    addTech("dimensional_sensors", "Dimensional Sensors", TechCategory::SENSORS, TechEra::FUTURE,
            6500, {"quantum_sensors", "phase_shields"}, "Detect objects via multidimensional signatures");
    addTech("terraform_tech", "Terraforming", TechCategory::BIOLOGY, TechEra::FUTURE,
            6000, {"orbital_construction"}, "Transform planetary environments");
    addTech("bioforming", "Bioforming", TechCategory::BIOLOGY, TechEra::FUTURE,
            6500, {"terraform_tech", "genetic_adaptation"}, "Seed and maintain engineered ecosystems");
    addTech("hyperspatial_logistics", "Hyperspatial Logistics", TechCategory::LOGISTICS, TechEra::FUTURE,
            7500, {"fleet_coordination_ai", "transwarp_drive"}, "Near-instant routing and resupply planning");
    addTech("self_repairing_hulls", "Self-Repairing Hulls", TechCategory::CONSTRUCTION, TechEra::FUTURE,
            7000, {"nanomaterials", "zero_point_energy"}, "Autonomous damage repair using embedded nanotech");
    addTech("omega_mining", "Omega Mining", TechCategory::MINING, TechEra::FUTURE,
            6000, {"deep_core_mining", "zero_point_energy"}, "Ultra-efficient extraction using exotic energy sources");
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

std::vector<std::shared_ptr<Technology>> ResearchTree::getAllTechs() const {
        std::vector<std::shared_ptr<Technology>> all;
        all.reserve(technologies.size());
        for (const auto& pair : technologies) {
                all.push_back(pair.second);
        }
        return all;
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

void ResearchTree::setTechStateForLoad(const std::string& techId, int progress, bool researchedFlag) {
        auto it = technologies.find(techId);
        if (it == technologies.end()) return;

        auto tech = it->second;
        if (!tech) return;

        tech->setProgressForLoad(progress);
        tech->setResearchedForLoad(researchedFlag);

        if (researchedFlag) {
                researched.insert(techId);
        } else {
                researched.erase(techId);
        }
}
