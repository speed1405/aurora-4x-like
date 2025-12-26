"""
Star system and galaxy generation.
"""

import random


class CelestialBody:
    """Base class for celestial bodies"""
    
    def __init__(self, name, body_type):
        self.name = name
        self.body_type = body_type


class Planet(CelestialBody):
    """Represents a planet"""
    
    TYPES = ["Terrestrial", "Gas Giant", "Ice", "Desert", "Ocean", "Volcanic"]
    
    def __init__(self, name, planet_type=None, minerals=None):
        super().__init__(name, "Planet")
        self.planet_type = planet_type or random.choice(self.TYPES)
        self.minerals = minerals or self._generate_minerals()
        self.colonized = False
        self.colony = None
    
    def _generate_minerals(self):
        """Generate random mineral deposits"""
        from src.resources import ResourceType
        minerals = {}
        
        # Aurora 4X style minerals
        mineral_types = [
            ResourceType.DURANIUM,
            ResourceType.NEUTRONIUM,
            ResourceType.CORUNDIUM,
            ResourceType.TRITANIUM,
            ResourceType.BORONIDE,
            ResourceType.MERCASSIUM,
            ResourceType.VENDARITE,
            ResourceType.SORIUM,
            ResourceType.URIDIUM,
            ResourceType.GALLICITE,
        ]
        
        for mineral in mineral_types:
            if random.random() > 0.3:  # 70% chance to have each mineral
                minerals[mineral] = random.randint(1000, 100000)
        
        return minerals
    
    def colonize(self, colony):
        """Set colony on planet"""
        self.colonized = True
        self.colony = colony


class Star:
    """Represents a star"""
    
    TYPES = ["Red Dwarf", "Yellow Dwarf", "Blue Giant", "Red Giant", "White Dwarf"]
    
    def __init__(self, name, star_type=None):
        self.name = name
        self.star_type = star_type or random.choice(self.TYPES)


class StarSystem:
    """Represents a star system"""
    
    def __init__(self, name, x=0, y=0, z=0):
        self.name = name
        self.x = x
        self.y = y
        self.z = z
        self.star = Star(f"{name} Primary")
        self.planets = []
        self.explored = False
        self._generate_planets()
    
    def _generate_planets(self):
        """Generate planets in the system"""
        num_planets = random.randint(2, 10)
        for i in range(num_planets):
            planet_name = f"{self.name} {chr(65 + i)}"  # A, B, C, etc.
            planet = Planet(planet_name)
            self.planets.append(planet)
    
    def explore(self):
        """Mark system as explored"""
        self.explored = True
    
    def get_colonizable_planets(self):
        """Get list of planets suitable for colonization"""
        return [p for p in self.planets 
                if p.planet_type in ["Terrestrial", "Ocean"] and not p.colonized]


class Galaxy:
    """Represents the galaxy/game map"""
    
    def __init__(self, num_systems=20):
        self.systems = []
        self.home_system = None
        self._generate_galaxy(num_systems)
    
    def _generate_galaxy(self, num_systems):
        """Generate star systems"""
        # Create home system
        self.home_system = StarSystem("Sol", 0, 0, 0)
        self.home_system.explored = True
        self.systems.append(self.home_system)
        
        # Generate other systems
        for i in range(num_systems - 1):
            name = self._generate_star_name(i)
            x = random.randint(-50, 50)
            y = random.randint(-50, 50)
            z = random.randint(-20, 20)
            system = StarSystem(name, x, y, z)
            self.systems.append(system)
    
    def _generate_star_name(self, index):
        """Generate a star name"""
        prefixes = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", 
                   "Theta", "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron"]
        suffixes = ["Centauri", "Draconis", "Eridani", "Cygni", "Leonis", "Aquarii",
                   "Cassiopeiae", "Orionis", "Pegasi", "Andromedae"]
        
        prefix = random.choice(prefixes)
        suffix = random.choice(suffixes)
        return f"{prefix} {suffix}"
    
    def get_explored_systems(self):
        """Get list of explored systems"""
        return [s for s in self.systems if s.explored]
    
    def get_unexplored_systems(self):
        """Get list of unexplored systems"""
        return [s for s in self.systems if not s.explored]
