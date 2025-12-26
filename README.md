# Aurora 4X-Like Space Empire Game

A space empire building game inspired by Aurora 4X, featuring:
- **Research System**: Progress from pre-warp to future technologies
- **Empire Building**: Manage resources, colonies, and populations
- **Combat**: Space fleet battles with strategic ship design
- **Exploration**: Discover and colonize new star systems
- **Resource Management**: Mine minerals and manage energy production

## Features

### Technology Research Tree
The game includes a comprehensive tech tree spanning five eras:

1. **Pre-Warp Era**: Basic mining, nuclear power, ion drives, missiles, basic sensors
2. **Early Warp Era**: First FTL drives (Warp 1), fusion power, laser weapons, basic shields
3. **Interstellar Era**: Improved warp drives (Warp 2), plasma weapons, advanced mining, orbital construction
4. **Advanced Era**: Warp 3, antimatter power, particle beams, graviton shields, quantum sensors
5. **Future Era**: Transwarp drives, zero-point energy, singularity weapons, phase shields, terraforming

Each technology has prerequisites and requires research points to unlock.

### Empire Management
- Manage your civilization's resources including minerals, energy, and research points
- Grow your population across multiple colonies
- Track empire statistics and progress

### Resource System
The game features Aurora 4X-style resources:
- **Strategic Resources**: Minerals, Energy, Research Points, Fuel
- **Minerals**: Duranium, Neutronium, Corundium, Tritanium, Boronide, Mercassium, Vendarite, Sorium, Uridium, Gallicite

### Fleet Combat
- Multiple ship classes: Scout, Fighter, Corvette, Frigate, Destroyer, Cruiser, Battleship, Carrier
- Weapon systems with damage, accuracy, and range
- Ships have hull and shields
- Turn-based combat resolution

### Galaxy Exploration
- Procedurally generated star systems with multiple planets
- Different planet types: Terrestrial, Gas Giant, Ice, Desert, Ocean, Volcanic
- Mineral deposits on planets for mining
- Colonization of suitable planets

## Installation

No external dependencies required! The game uses only C++ standard library.

### Requirements
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher

### Building

```bash
# Clone the repository
git clone https://github.com/speed1405/aurora-4x-like.git
cd aurora-4x-like

# Build the game
mkdir build
cd build
cmake ..
make

# Run the game
./aurora4x
```

### Windows Build

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Run the game
.\Release\aurora4x.exe
```

## How to Play

### Starting the Game
1. Run `./aurora4x` (Linux/Mac) or `aurora4x.exe` (Windows)
2. Enter your empire name or press Enter for default "Earth Empire"
3. Start managing your space empire!

### Game Loop
The game is turn-based. Each turn:
- Resources are produced automatically
- Research progresses if a technology is selected
- Population grows on colonies

### Main Menu Options

**1. Empire Status**
- View your empire's current state
- Check resources, colonies, fleets
- See research progress

**2. Research**
- View available technologies
- Start researching new technologies
- Technologies unlock based on prerequisites

**3. Explore Galaxy**
- View explored and unexplored star systems
- Explore new systems to discover planets
- Find colonization opportunities

**4. Fleet Management**
- View your fleets and ships
- Build new ships (Scout, Corvette, Frigate)
- Check ship status and combat readiness

**5. Combat Simulation**
- Simulate battles between your fleets
- Test ship designs and strategies
- View detailed combat logs

**6. Advance Turn**
- Progress the game one turn
- Generate resources
- Advance research

**7. Help**
- View in-game help and instructions

### Research Strategy
1. Start with basic technologies that have no prerequisites
2. Plan your tech path based on your strategy (military, economic, or exploration focus)
3. Allocate research points each turn to progress technologies
4. Higher-tier technologies require more research points and have more prerequisites

### Combat Tips
- Scouts are fast but weak
- Corvettes are balanced light warships
- Frigates are heavier with more firepower
- Combat is resolved in rounds with simultaneous attacks
- Ships with shields absorb damage before hull takes damage

## Game Mechanics

### Resource Production
- **Minerals**: 10 per turn (base)
- **Energy**: 50 per turn (base)
- **Research Points**: 5 per turn (base)

### Technology Costs
- Pre-Warp: 100-200 research points
- Early Warp: 500-1000 research points
- Interstellar: 1200-2000 research points
- Advanced: 2500-4000 research points
- Future: 6000-10000 research points

### Ship Classes
| Class | Hull | Shields | Weapons |
|-------|------|---------|---------|
| Scout | 50 | 20 | 1 Laser |
| Corvette | 100 | 50 | 2 Lasers |
| Frigate | 200 | 100 | 3 Lasers |

## Project Structure

```
aurora-4x-like/
├── CMakeLists.txt       # CMake build configuration
├── include/             # Header files
│   ├── game.h          # Main game controller
│   ├── empire.h        # Empire and colony management
│   ├── research.h      # Technology and research tree
│   ├── resources.h     # Resource management system
│   ├── combat.h        # Combat system and ships
│   └── galaxy.h        # Star systems and galaxy generation
├── src/                # Implementation files
│   ├── main.cpp        # Main game entry point and CLI
│   ├── game.cpp        # Main game controller implementation
│   ├── empire.cpp      # Empire management implementation
│   ├── research.cpp    # Research tree implementation
│   ├── resources.cpp   # Resource system implementation
│   ├── combat.cpp      # Combat system implementation
│   └── galaxy.cpp      # Galaxy generation implementation
├── .gitignore          # Git ignore rules
└── README.md           # This file
```

## Code Architecture

The game is written in modern C++17 with a modular design.

### Core Modules

**game.h/cpp**: Main game controller that ties all systems together
- Initializes empire, galaxy, and starting conditions
- Provides high-level game operations
- Manages turn progression

**empire.h/cpp**: Empire and colony management
- `Empire`: Player's civilization with resources, research, and fleets
- `Colony`: Individual colony on a planet

**research.h/cpp**: Technology system
- `Technology`: Individual tech with prerequisites and costs
- `ResearchTree`: Manages entire tech tree and research progress
- Organized by category and era

**resources.h/cpp**: Resource management
- `ResourceStorage`: Tracks and manages all resources
- `ResourceNode`: Represents mineral deposits
- Production and consumption mechanics

**combat.h/cpp**: Space combat system
- `Ship`: Individual ships with weapons, hull, and shields
- `Fleet`: Collections of ships
- `Combat`: Resolves battles between fleets
- `Weapon`: Weapon systems with damage and accuracy

**galaxy.h/cpp**: Universe generation
- `Galaxy`: Overall game map
- `StarSystem`: Individual star systems with planets
- `Planet`: Planets with types and mineral deposits
- `Star`: Star types

**main.cpp**: Command-line interface
- Interactive menu system
- Display functions
- User input handling

## Future Enhancements

Possible additions for future versions:
- Save/Load game functionality
- More ship classes and weapon types
- Diplomacy and AI empires
- Trade and economy systems
- More complex colony management
- Ship design system
- Space stations and megastructures
- Random events and anomalies
- Mission system
- Victory conditions

## Contributing

Feel free to fork and submit pull requests! Areas that could use improvement:
- Balance tweaking for technologies and combat
- Additional ship classes and weapons
- UI improvements
- More strategic depth
- Additional events and content

## License

Open source - feel free to use and modify!

## Credits

Inspired by Aurora 4X by Steve Walmsley, one of the deepest space empire simulation games ever created.