"""
Technology and Research system.
Handles tech tree from pre-warp to advanced future technologies.
"""

class TechCategory:
    """Technology categories"""
    PROPULSION = "Propulsion"
    WEAPONS = "Weapons"
    SENSORS = "Sensors"
    SHIELDS = "Shields"
    CONSTRUCTION = "Construction"
    MINING = "Mining"
    POWER = "Power Generation"
    BIOLOGY = "Biology"
    LOGISTICS = "Logistics"


class TechEra:
    """Technology eras"""
    PRE_WARP = "Pre-Warp"
    EARLY_WARP = "Early Warp"
    INTERSTELLAR = "Interstellar"
    ADVANCED = "Advanced"
    FUTURE = "Future"


class Technology:
    """Represents a single technology"""
    
    def __init__(self, tech_id, name, category, era, cost, prerequisites=None, 
                 description="", effects=None):
        self.tech_id = tech_id
        self.name = name
        self.category = category
        self.era = era
        self.cost = cost  # Research points needed
        self.prerequisites = prerequisites or []
        self.description = description
        self.effects = effects or {}
        self.researched = False
        self.progress = 0
    
    def is_available(self, researched_techs):
        """Check if all prerequisites are met"""
        return all(prereq in researched_techs for prereq in self.prerequisites)
    
    def add_progress(self, points):
        """Add research progress. Returns True if completed"""
        if self.researched:
            return True
        self.progress += points
        if self.progress >= self.cost:
            self.researched = True
            return True
        return False


class ResearchTree:
    """Manages the technology tree"""
    
    def __init__(self):
        self.technologies = {}
        self.researched = set()
        self._initialize_tech_tree()
    
    def _initialize_tech_tree(self):
        """Initialize the technology tree"""
        
        # Pre-Warp Era
        self._add_tech("basic_mining", "Basic Mining", TechCategory.MINING, TechEra.PRE_WARP, 
                      100, [], "Unlocks basic mining facilities")
        
        self._add_tech("nuclear_power", "Nuclear Power", TechCategory.POWER, TechEra.PRE_WARP,
                      150, [], "Nuclear power generation")
        
        self._add_tech("ion_drive", "Ion Drive", TechCategory.PROPULSION, TechEra.PRE_WARP,
                      200, [], "Basic ion propulsion for in-system travel")
        
        self._add_tech("missile_tech", "Missile Technology", TechCategory.WEAPONS, TechEra.PRE_WARP,
                      150, [], "Basic missile weapons")
        
        self._add_tech("basic_sensors", "Basic Sensors", TechCategory.SENSORS, TechEra.PRE_WARP,
                      100, [], "Short-range detection systems")
        
        # Early Warp Era
        self._add_tech("warp_theory", "Warp Theory", TechCategory.PROPULSION, TechEra.EARLY_WARP,
                      500, ["ion_drive", "nuclear_power"], "Theoretical basis for FTL travel")
        
        self._add_tech("warp_drive_1", "Warp Drive I", TechCategory.PROPULSION, TechEra.EARLY_WARP,
                      1000, ["warp_theory"], "First generation FTL drive, Warp 1")
        
        self._add_tech("fusion_power", "Fusion Power", TechCategory.POWER, TechEra.EARLY_WARP,
                      600, ["nuclear_power"], "Fusion reactor technology")
        
        self._add_tech("laser_weapons", "Laser Weapons", TechCategory.WEAPONS, TechEra.EARLY_WARP,
                      700, ["missile_tech"], "Energy-based weapons")
        
        self._add_tech("basic_shields", "Basic Shields", TechCategory.SHIELDS, TechEra.EARLY_WARP,
                      800, ["fusion_power"], "Protective energy shields")
        
        # Interstellar Era
        self._add_tech("warp_drive_2", "Warp Drive II", TechCategory.PROPULSION, TechEra.INTERSTELLAR,
                      2000, ["warp_drive_1"], "Improved FTL drive, Warp 2")
        
        self._add_tech("advanced_mining", "Advanced Mining", TechCategory.MINING, TechEra.INTERSTELLAR,
                      1500, ["basic_mining", "fusion_power"], "Automated mining systems")
        
        self._add_tech("plasma_weapons", "Plasma Weapons", TechCategory.WEAPONS, TechEra.INTERSTELLAR,
                      1800, ["laser_weapons", "fusion_power"], "Superheated plasma cannons")
        
        self._add_tech("long_range_sensors", "Long Range Sensors", TechCategory.SENSORS, TechEra.INTERSTELLAR,
                      1200, ["basic_sensors"], "Extended detection range")
        
        self._add_tech("advanced_shields", "Advanced Shields", TechCategory.SHIELDS, TechEra.INTERSTELLAR,
                      2000, ["basic_shields"], "Improved shield strength")
        
        self._add_tech("orbital_construction", "Orbital Construction", TechCategory.CONSTRUCTION, 
                      TechEra.INTERSTELLAR, 1600, ["advanced_mining"], "Build stations in space")
        
        # Advanced Era
        self._add_tech("warp_drive_3", "Warp Drive III", TechCategory.PROPULSION, TechEra.ADVANCED,
                      4000, ["warp_drive_2"], "Advanced FTL drive, Warp 3")
        
        self._add_tech("antimatter_power", "Antimatter Power", TechCategory.POWER, TechEra.ADVANCED,
                      3500, ["fusion_power"], "Antimatter reactors")
        
        self._add_tech("particle_beam", "Particle Beam Weapons", TechCategory.WEAPONS, TechEra.ADVANCED,
                      3000, ["plasma_weapons"], "Particle accelerator weapons")
        
        self._add_tech("graviton_shields", "Graviton Shields", TechCategory.SHIELDS, TechEra.ADVANCED,
                      3500, ["advanced_shields"], "Gravity-based shields")
        
        self._add_tech("quantum_sensors", "Quantum Sensors", TechCategory.SENSORS, TechEra.ADVANCED,
                      2500, ["long_range_sensors"], "Quantum entanglement detection")
        
        # Future Era
        self._add_tech("transwarp_drive", "Transwarp Drive", TechCategory.PROPULSION, TechEra.FUTURE,
                      8000, ["warp_drive_3", "antimatter_power"], "Experimental ultra-fast FTL")
        
        self._add_tech("zero_point_energy", "Zero Point Energy", TechCategory.POWER, TechEra.FUTURE,
                      7000, ["antimatter_power"], "Tap vacuum energy")
        
        self._add_tech("singularity_weapons", "Singularity Weapons", TechCategory.WEAPONS, TechEra.FUTURE,
                      10000, ["particle_beam", "graviton_shields"], "Micro black hole weapons")
        
        self._add_tech("phase_shields", "Phase Shields", TechCategory.SHIELDS, TechEra.FUTURE,
                      9000, ["graviton_shields"], "Dimensional phase shifting")
        
        self._add_tech("terraform_tech", "Terraforming", TechCategory.BIOLOGY, TechEra.FUTURE,
                      6000, ["orbital_construction"], "Transform planetary environments")
    
    def _add_tech(self, tech_id, name, category, era, cost, prerequisites, description):
        """Helper to add technology to tree"""
        tech = Technology(tech_id, name, category, era, cost, prerequisites, description)
        self.technologies[tech_id] = tech
    
    def get_available_techs(self):
        """Get list of technologies available for research"""
        available = []
        for tech_id, tech in self.technologies.items():
            if not tech.researched and tech.is_available(self.researched):
                available.append(tech)
        return available
    
    def research(self, tech_id, points):
        """Add research points to a technology. Returns True if completed"""
        if tech_id not in self.technologies:
            return False
        
        tech = self.technologies[tech_id]
        if not tech.is_available(self.researched):
            return False
        
        completed = tech.add_progress(points)
        if completed:
            self.researched.add(tech_id)
        return completed
    
    def get_tech(self, tech_id):
        """Get technology by ID"""
        return self.technologies.get(tech_id)
    
    def is_researched(self, tech_id):
        """Check if technology is researched"""
        return tech_id in self.researched
    
    def get_researched_count(self):
        """Get count of researched technologies"""
        return len(self.researched)
