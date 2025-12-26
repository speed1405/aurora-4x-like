"""
Resource management system for the game.
Handles minerals, energy, population, and other resources.
"""

class ResourceType:
    """Enum-like class for resource types"""
    MINERALS = "minerals"
    ENERGY = "energy"
    POPULATION = "population"
    RESEARCH_POINTS = "research_points"
    FUEL = "fuel"
    DURANIUM = "duranium"
    NEUTRONIUM = "neutronium"
    CORUNDIUM = "corundium"
    TRITANIUM = "tritanium"
    BORONIDE = "boronide"
    MERCASSIUM = "mercassium"
    VENDARITE = "vendarite"
    SORIUM = "sorium"
    URIDIUM = "uridium"
    GALLICITE = "gallicite"


class ResourceStorage:
    """Manages resource storage and production"""
    
    def __init__(self):
        self.resources = {
            ResourceType.MINERALS: 0,
            ResourceType.ENERGY: 0,
            ResourceType.POPULATION: 100,  # Start with base population
            ResourceType.RESEARCH_POINTS: 0,
            ResourceType.FUEL: 1000,
            ResourceType.DURANIUM: 500,
            ResourceType.NEUTRONIUM: 200,
            ResourceType.CORUNDIUM: 100,
            ResourceType.TRITANIUM: 150,
            ResourceType.BORONIDE: 50,
            ResourceType.MERCASSIUM: 50,
            ResourceType.VENDARITE: 30,
            ResourceType.SORIUM: 100,
            ResourceType.URIDIUM: 20,
            ResourceType.GALLICITE: 40,
        }
        self.production_rates = {
            ResourceType.MINERALS: 10,
            ResourceType.ENERGY: 50,
            ResourceType.RESEARCH_POINTS: 5,
        }
    
    def get(self, resource_type):
        """Get current amount of a resource"""
        return self.resources.get(resource_type, 0)
    
    def add(self, resource_type, amount):
        """Add resources"""
        if resource_type not in self.resources:
            self.resources[resource_type] = 0
        self.resources[resource_type] += amount
    
    def consume(self, resource_type, amount):
        """Consume resources. Returns True if successful, False if insufficient"""
        if self.resources.get(resource_type, 0) >= amount:
            self.resources[resource_type] -= amount
            return True
        return False
    
    def produce(self, turns=1):
        """Produce resources for given number of turns"""
        for resource_type, rate in self.production_rates.items():
            self.add(resource_type, rate * turns)
    
    def can_afford(self, costs):
        """Check if empire can afford given costs (dict of resource_type: amount)"""
        for resource_type, amount in costs.items():
            if self.get(resource_type) < amount:
                return False
        return True
    
    def pay_costs(self, costs):
        """Pay costs if possible. Returns True if successful"""
        if not self.can_afford(costs):
            return False
        for resource_type, amount in costs.items():
            self.consume(resource_type, amount)
        return True


class ResourceNode:
    """Represents a resource deposit on a planet or asteroid"""
    
    def __init__(self, resource_type, amount, extraction_rate=1.0):
        self.resource_type = resource_type
        self.amount = amount
        self.extraction_rate = extraction_rate
    
    def extract(self, capacity):
        """Extract resources up to capacity. Returns amount extracted"""
        extractable = min(self.amount, capacity * self.extraction_rate)
        self.amount -= extractable
        return extractable
