"""
Main game class that ties everything together.
"""

from src.empire import Empire, Colony
from src.galaxy import Galaxy
from src.combat import Ship, Fleet, Combat, Weapon, ShipClass
from src.resources import ResourceType


class Game:
    """Main game controller"""
    
    def __init__(self, empire_name="Earth Empire"):
        self.empire = Empire(empire_name)
        self.galaxy = Galaxy(num_systems=20)
        self.running = False
        
        # Setup starting conditions
        self._setup_game()
    
    def _setup_game(self):
        """Setup initial game state"""
        # Colonize home planet (Earth)
        home_planet = self.galaxy.home_system.planets[2]  # 3rd planet
        home_planet.planet_type = "Terrestrial"
        earth_colony = Colony("Earth", home_planet)
        earth_colony.population = 100
        home_planet.colonize(earth_colony)
        self.empire.add_colony(earth_colony)
        
        # Create starting fleet
        starting_fleet = self._create_starting_fleet()
        self.empire.add_fleet(starting_fleet)
    
    def _create_starting_fleet(self):
        """Create the starting fleet"""
        fleet = Fleet("Home Defense Fleet", self.empire.name)
        
        # Add basic ships
        laser = Weapon("Laser Cannon", damage=10, accuracy=0.7, range_val=5)
        
        scout = Ship("Scout-1", ShipClass.SCOUT, hull=50, shields=20, weapons=[laser])
        corvette = Ship("Corvette-1", ShipClass.CORVETTE, hull=100, shields=50, 
                       weapons=[laser, laser])
        
        fleet.add_ship(scout)
        fleet.add_ship(corvette)
        fleet.location = self.galaxy.home_system
        
        return fleet
    
    def advance_turn(self):
        """Advance the game one turn"""
        message = self.empire.advance_turn()
        
        # Colony growth
        for colony in self.empire.colonies:
            colony.grow()
        
        return message
    
    def explore_system(self, system_name):
        """Explore a star system"""
        for system in self.galaxy.systems:
            if system.name.lower() == system_name.lower():
                system.explore()
                return f"Explored {system.name}! Found {len(system.planets)} planets."
        return "System not found"
    
    def start_research(self, tech_id):
        """Start researching a technology"""
        if self.empire.set_research(tech_id):
            tech = self.empire.research.get_tech(tech_id)
            return f"Now researching: {tech.name}"
        return "Cannot research that technology"
    
    def get_available_research(self):
        """Get list of available technologies to research"""
        return self.empire.research.get_available_techs()
    
    def build_ship(self, ship_class, fleet_name):
        """Build a new ship and add to fleet"""
        # Find fleet
        fleet = None
        for f in self.empire.fleets:
            if f.name.lower() == fleet_name.lower():
                fleet = f
                break
        
        if not fleet:
            return "Fleet not found"
        
        # Simple ship creation (costs not implemented yet)
        ship_name = f"{ship_class}-{len(fleet.ships) + 1}"
        
        if ship_class == ShipClass.SCOUT:
            laser = Weapon("Laser", 10, 0.7, 5)
            ship = Ship(ship_name, ship_class, 50, 20, [laser])
        elif ship_class == ShipClass.CORVETTE:
            laser = Weapon("Laser", 10, 0.7, 5)
            ship = Ship(ship_name, ship_class, 100, 50, [laser, laser])
        elif ship_class == ShipClass.FRIGATE:
            laser = Weapon("Laser", 15, 0.75, 6)
            ship = Ship(ship_name, ship_class, 200, 100, [laser, laser, laser])
        else:
            return "Unknown ship class"
        
        fleet.add_ship(ship)
        return f"Built {ship_name} and added to {fleet_name}"
    
    def simulate_combat(self, fleet1_name, fleet2_name):
        """Simulate combat between two fleets"""
        fleet1 = None
        fleet2 = None
        
        for fleet in self.empire.fleets:
            if fleet.name.lower() == fleet1_name.lower():
                fleet1 = fleet
            elif fleet.name.lower() == fleet2_name.lower():
                fleet2 = fleet
        
        if not fleet1 or not fleet2:
            return "One or both fleets not found"
        
        combat = Combat(fleet1, fleet2)
        winner = combat.resolve()
        
        return combat.get_log()
    
    def get_empire_status(self):
        """Get current empire status"""
        status = self.empire.get_status()
        
        # Add resource info
        status["resources"] = {
            "minerals": self.empire.resources.get(ResourceType.MINERALS),
            "energy": self.empire.resources.get(ResourceType.ENERGY),
            "population": self.empire.resources.get(ResourceType.POPULATION),
            "research_points": self.empire.resources.get(ResourceType.RESEARCH_POINTS),
        }
        
        return status
    
    def list_systems(self):
        """List all star systems"""
        return self.galaxy.systems
    
    def list_fleets(self):
        """List all fleets"""
        return self.empire.fleets
