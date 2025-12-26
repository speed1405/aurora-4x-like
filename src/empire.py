"""
Empire management system.
Handles empire state, colonies, and overall civilization management.
"""

from src.resources import ResourceStorage
from src.research import ResearchTree


class Empire:
    """Represents a player's empire/civilization"""
    
    def __init__(self, name="Earth Empire"):
        self.name = name
        self.resources = ResourceStorage()
        self.research = ResearchTree()
        self.colonies = []
        self.fleets = []
        self.turn = 0
        self.current_research = None
        
        # Empire stats
        self.total_population = 100
        self.military_strength = 0
    
    def advance_turn(self):
        """Advance one game turn"""
        self.turn += 1
        
        # Produce resources
        self.resources.produce()
        
        # Research progress
        if self.current_research:
            research_points = self.resources.get("research_points")
            if research_points > 0:
                # Allocate research points
                points_to_use = min(research_points, 10)  # Use up to 10 per turn
                completed = self.research.research(self.current_research, points_to_use)
                self.resources.consume("research_points", points_to_use)
                
                if completed:
                    tech = self.research.get_tech(self.current_research)
                    self.current_research = None
                    return f"Research completed: {tech.name}!"
        
        return f"Turn {self.turn} completed"
    
    def set_research(self, tech_id):
        """Set current research target"""
        tech = self.research.get_tech(tech_id)
        if tech and not tech.researched and tech.is_available(self.research.researched):
            self.current_research = tech_id
            return True
        return False
    
    def add_colony(self, colony):
        """Add a colony to the empire"""
        self.colonies.append(colony)
    
    def add_fleet(self, fleet):
        """Add a fleet to the empire"""
        self.fleets.append(fleet)
    
    def get_status(self):
        """Get empire status summary"""
        status = {
            "name": self.name,
            "turn": self.turn,
            "population": self.total_population,
            "colonies": len(self.colonies),
            "fleets": len(self.fleets),
            "researched_techs": self.research.get_researched_count(),
            "current_research": self.current_research,
        }
        return status


class Colony:
    """Represents a colony on a planet"""
    
    def __init__(self, name, planet):
        self.name = name
        self.planet = planet
        self.population = 10
        self.infrastructure = 1
        self.mines = 0
        self.factories = 0
    
    def grow(self):
        """Grow colony population"""
        growth_rate = 0.01 * self.infrastructure
        self.population += int(self.population * growth_rate)
    
    def build_mine(self):
        """Build a mine"""
        self.mines += 1
    
    def build_factory(self):
        """Build a factory"""
        self.factories += 1
