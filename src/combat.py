"""
Combat system for space battles.
"""

import random


class ShipClass:
    """Ship class definitions"""
    SCOUT = "Scout"
    FIGHTER = "Fighter"
    CORVETTE = "Corvette"
    FRIGATE = "Frigate"
    DESTROYER = "Destroyer"
    CRUISER = "Cruiser"
    BATTLESHIP = "Battleship"
    CARRIER = "Carrier"


class Weapon:
    """Weapon system"""
    
    def __init__(self, name, damage, accuracy, range_val):
        self.name = name
        self.damage = damage
        self.accuracy = accuracy  # 0.0 to 1.0
        self.range = range_val
    
    def fire(self, target):
        """Fire at target. Returns damage dealt"""
        if random.random() < self.accuracy:
            return self.damage
        return 0


class Ship:
    """Represents a single ship"""
    
    def __init__(self, name, ship_class, hull, shields, weapons=None):
        self.name = name
        self.ship_class = ship_class
        self.max_hull = hull
        self.hull = hull
        self.max_shields = shields
        self.shields = shields
        self.weapons = weapons or []
        self.destroyed = False
    
    def take_damage(self, damage):
        """Apply damage to ship"""
        # Shields absorb first
        if self.shields > 0:
            absorbed = min(self.shields, damage)
            self.shields -= absorbed
            damage -= absorbed
        
        # Remaining damage to hull
        if damage > 0:
            self.hull -= damage
            if self.hull <= 0:
                self.hull = 0
                self.destroyed = True
    
    def fire_at(self, target):
        """Fire all weapons at target"""
        total_damage = 0
        for weapon in self.weapons:
            total_damage += weapon.fire(target)
        return total_damage
    
    def is_operational(self):
        """Check if ship is still operational"""
        return not self.destroyed and self.hull > 0


class Fleet:
    """Represents a fleet of ships"""
    
    def __init__(self, name, owner):
        self.name = name
        self.owner = owner
        self.ships = []
        self.location = None
    
    def add_ship(self, ship):
        """Add ship to fleet"""
        self.ships.append(ship)
    
    def remove_destroyed(self):
        """Remove destroyed ships"""
        self.ships = [ship for ship in self.ships if ship.is_operational()]
    
    def get_combat_strength(self):
        """Calculate total combat strength"""
        strength = 0
        for ship in self.ships:
            if ship.is_operational():
                strength += ship.hull + ship.shields
        return strength
    
    def is_defeated(self):
        """Check if fleet is defeated"""
        return all(not ship.is_operational() for ship in self.ships)


class Combat:
    """Handles combat between fleets"""
    
    def __init__(self, attacker, defender):
        self.attacker = attacker
        self.defender = defender
        self.combat_log = []
        self.round = 0
    
    def resolve_round(self):
        """Resolve one combat round"""
        self.round += 1
        self.combat_log.append(f"=== Combat Round {self.round} ===")
        
        # Attacker fires
        for ship in self.attacker.ships:
            if ship.is_operational() and self.defender.ships:
                target = random.choice([s for s in self.defender.ships if s.is_operational()])
                if target:
                    damage = ship.fire_at(target)
                    if damage > 0:
                        target.take_damage(damage)
                        self.combat_log.append(
                            f"{ship.name} hits {target.name} for {damage} damage"
                        )
                        if target.destroyed:
                            self.combat_log.append(f"{target.name} destroyed!")
        
        # Defender fires back
        for ship in self.defender.ships:
            if ship.is_operational() and self.attacker.ships:
                target = random.choice([s for s in self.attacker.ships if s.is_operational()])
                if target:
                    damage = ship.fire_at(target)
                    if damage > 0:
                        target.take_damage(damage)
                        self.combat_log.append(
                            f"{ship.name} hits {target.name} for {damage} damage"
                        )
                        if target.destroyed:
                            self.combat_log.append(f"{target.name} destroyed!")
        
        # Remove destroyed ships
        self.attacker.remove_destroyed()
        self.defender.remove_destroyed()
    
    def resolve(self, max_rounds=10):
        """Resolve combat until one side is defeated or max rounds reached"""
        while self.round < max_rounds:
            self.resolve_round()
            
            if self.attacker.is_defeated():
                self.combat_log.append(f"{self.defender.name} wins!")
                return self.defender
            elif self.defender.is_defeated():
                self.combat_log.append(f"{self.attacker.name} wins!")
                return self.attacker
        
        # If max rounds reached, check who has more strength
        attacker_strength = self.attacker.get_combat_strength()
        defender_strength = self.defender.get_combat_strength()
        
        if attacker_strength > defender_strength:
            self.combat_log.append(f"{self.attacker.name} wins by attrition!")
            return self.attacker
        else:
            self.combat_log.append(f"{self.defender.name} wins by attrition!")
            return self.defender
    
    def get_log(self):
        """Get combat log"""
        return "\n".join(self.combat_log)
