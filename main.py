#!/usr/bin/env python3
"""
Aurora 4X-like Space Empire Game
Command-line interface for the game
"""

import sys
from src.game import Game


def print_header():
    """Print game header"""
    print("=" * 60)
    print("AURORA 4X-LIKE: Space Empire Builder".center(60))
    print("=" * 60)
    print()


def print_menu():
    """Print main menu"""
    print("\n" + "=" * 60)
    print("MAIN MENU".center(60))
    print("=" * 60)
    print("1. Empire Status")
    print("2. Research")
    print("3. Explore Galaxy")
    print("4. Fleet Management")
    print("5. Combat Simulation")
    print("6. Advance Turn")
    print("7. Help")
    print("0. Exit")
    print("=" * 60)


def display_empire_status(game):
    """Display empire status"""
    status = game.get_empire_status()
    
    print("\n" + "=" * 60)
    print(f"EMPIRE: {status['name']}")
    print("=" * 60)
    print(f"Turn: {status['turn']}")
    print(f"Colonies: {status['colonies']}")
    print(f"Fleets: {status['fleets']}")
    print(f"Researched Technologies: {status['researched_techs']}")
    print(f"Current Research: {status['current_research'] or 'None'}")
    print("\nResources:")
    for resource, amount in status['resources'].items():
        print(f"  {resource}: {amount}")
    print("=" * 60)


def research_menu(game):
    """Research management menu"""
    while True:
        print("\n" + "=" * 60)
        print("RESEARCH MENU".center(60))
        print("=" * 60)
        
        status = game.get_empire_status()
        print(f"Current Research: {status['current_research'] or 'None'}")
        print(f"Research Points Available: {status['resources']['research_points']}")
        
        available = game.get_available_research()
        
        if not available:
            print("\nNo technologies available for research.")
            print("(Prerequisites not met or all researched)")
        else:
            print("\nAvailable Technologies:")
            for i, tech in enumerate(available, 1):
                print(f"{i}. {tech.name} ({tech.category}, {tech.era})")
                print(f"   Cost: {tech.cost} RP, Progress: {tech.progress}/{tech.cost}")
                if tech.prerequisites:
                    print(f"   Prerequisites: {', '.join(tech.prerequisites)}")
        
        print("\nOptions:")
        print("R <number> - Start researching technology")
        print("B - Back to main menu")
        
        choice = input("\nChoice: ").strip().lower()
        
        if choice == 'b':
            break
        elif choice.startswith('r '):
            try:
                tech_num = int(choice.split()[1])
                if 1 <= tech_num <= len(available):
                    tech = available[tech_num - 1]
                    result = game.start_research(tech.tech_id)
                    print(f"\n{result}")
                else:
                    print("\nInvalid technology number")
            except (ValueError, IndexError):
                print("\nInvalid input")


def explore_menu(game):
    """Exploration menu"""
    print("\n" + "=" * 60)
    print("GALAXY EXPLORATION".center(60))
    print("=" * 60)
    
    systems = game.list_systems()
    
    print("\nExplored Systems:")
    for system in game.galaxy.get_explored_systems():
        print(f"  {system.name} ({system.x}, {system.y}, {system.z})")
        print(f"    Star: {system.star.star_type}")
        print(f"    Planets: {len(system.planets)}")
    
    print("\nUnexplored Systems:")
    unexplored = game.galaxy.get_unexplored_systems()
    for i, system in enumerate(unexplored[:10], 1):  # Show first 10
        print(f"  {i}. {system.name} ({system.x}, {system.y}, {system.z})")
    
    if len(unexplored) > 10:
        print(f"  ... and {len(unexplored) - 10} more")
    
    print("\nOptions:")
    print("E <number> - Explore system by number")
    print("B - Back")
    
    choice = input("\nChoice: ").strip().lower()
    
    if choice.startswith('e '):
        try:
            sys_num = int(choice.split()[1])
            if 1 <= sys_num <= len(unexplored):
                system = unexplored[sys_num - 1]
                result = game.explore_system(system.name)
                print(f"\n{result}")
                
                # Show planet details
                for planet in system.planets:
                    print(f"\n  Planet: {planet.name}")
                    print(f"    Type: {planet.planet_type}")
                    print(f"    Minerals: {len(planet.minerals)} types")
        except (ValueError, IndexError):
            print("\nInvalid input")


def fleet_menu(game):
    """Fleet management menu"""
    print("\n" + "=" * 60)
    print("FLEET MANAGEMENT".center(60))
    print("=" * 60)
    
    fleets = game.list_fleets()
    
    print("\nYour Fleets:")
    for i, fleet in enumerate(fleets, 1):
        print(f"\n{i}. {fleet.name}")
        print(f"   Location: {fleet.location.name if fleet.location else 'Unknown'}")
        print(f"   Ships: {len(fleet.ships)}")
        print(f"   Combat Strength: {fleet.get_combat_strength()}")
        
        if fleet.ships:
            print("   Ship List:")
            for ship in fleet.ships:
                print(f"     - {ship.name} ({ship.ship_class}): "
                      f"Hull {ship.hull}/{ship.max_hull}, "
                      f"Shields {ship.shields}/{ship.max_shields}")
    
    print("\nOptions:")
    print("B <class> <fleet_name> - Build ship (Scout, Corvette, Frigate)")
    print("M - Back to main menu")
    
    choice = input("\nChoice: ").strip().lower()
    
    if choice.startswith('b '):
        parts = choice.split(maxsplit=2)
        if len(parts) >= 3:
            ship_class = parts[1].capitalize()
            fleet_name = parts[2]
            result = game.build_ship(ship_class, fleet_name)
            print(f"\n{result}")


def combat_menu(game):
    """Combat simulation menu"""
    print("\n" + "=" * 60)
    print("COMBAT SIMULATION".center(60))
    print("=" * 60)
    
    fleets = game.list_fleets()
    
    if len(fleets) < 2:
        print("\nNeed at least 2 fleets for combat simulation")
        return
    
    print("\nAvailable Fleets:")
    for i, fleet in enumerate(fleets, 1):
        print(f"{i}. {fleet.name} (Strength: {fleet.get_combat_strength()})")
    
    print("\nEnter two fleet numbers to simulate combat (e.g., '1 2')")
    print("Or 'B' to go back")
    
    choice = input("\nChoice: ").strip()
    
    if choice.lower() == 'b':
        return
    
    try:
        nums = choice.split()
        if len(nums) == 2:
            idx1, idx2 = int(nums[0]) - 1, int(nums[1]) - 1
            if 0 <= idx1 < len(fleets) and 0 <= idx2 < len(fleets):
                fleet1 = fleets[idx1]
                fleet2 = fleets[idx2]
                
                print(f"\nSimulating combat between {fleet1.name} and {fleet2.name}...")
                result = game.simulate_combat(fleet1.name, fleet2.name)
                print("\n" + result)
    except (ValueError, IndexError):
        print("\nInvalid input")


def show_help():
    """Show help information"""
    print("\n" + "=" * 60)
    print("HELP".center(60))
    print("=" * 60)
    print("""
This is a space empire building game inspired by Aurora 4X.

GAMEPLAY OVERVIEW:
- Manage your empire's resources and population
- Research technologies from pre-warp to future eras
- Explore star systems and colonize planets
- Build fleets and engage in space combat

RESEARCH:
Technologies are organized into eras:
- Pre-Warp: Basic systems (ion drives, missiles, mining)
- Early Warp: First FTL capabilities
- Interstellar: Advanced space travel and weapons
- Advanced: Cutting-edge technologies
- Future: Experimental and theoretical tech

Each technology requires research points and may have prerequisites.

RESOURCES:
- Minerals: Used for construction
- Energy: Powers systems
- Research Points: Generated each turn for research
- Population: Grows over time on colonies

COMBAT:
Ships have hull and shields. Weapons have damage and accuracy.
Combat is resolved in rounds until one side is defeated.

EXPLORATION:
Explore star systems to find planets for colonization and resources.

Press ENTER to continue...
    """)
    input()


def main():
    """Main game loop"""
    print_header()
    
    # Get empire name
    empire_name = input("Enter your empire name (or press Enter for 'Earth Empire'): ").strip()
    if not empire_name:
        empire_name = "Earth Empire"
    
    # Create game
    game = Game(empire_name)
    
    print(f"\nWelcome, leader of {empire_name}!")
    print("Your civilization has just achieved spaceflight capability.")
    print("Guide your empire from pre-warp to the stars!")
    
    input("\nPress ENTER to begin...")
    
    # Main game loop
    while True:
        print_menu()
        choice = input("\nEnter your choice: ").strip()
        
        if choice == '0':
            print("\nThank you for playing!")
            break
        elif choice == '1':
            display_empire_status(game)
        elif choice == '2':
            research_menu(game)
        elif choice == '3':
            explore_menu(game)
        elif choice == '4':
            fleet_menu(game)
        elif choice == '5':
            combat_menu(game)
        elif choice == '6':
            result = game.advance_turn()
            print(f"\n{result}")
            
            # Show some info after turn
            status = game.get_empire_status()
            print(f"Resources: Minerals={status['resources']['minerals']}, "
                  f"Energy={status['resources']['energy']}, "
                  f"Research={status['resources']['research_points']}")
        elif choice == '7':
            show_help()
        else:
            print("\nInvalid choice. Please try again.")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nGame interrupted. Goodbye!")
        sys.exit(0)
