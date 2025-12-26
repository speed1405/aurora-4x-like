# Aurora 4X-Like Game - Quick Start Guide

## Building the Game

### Linux/Mac
```bash
mkdir build
cd build
cmake ..
make
./aurora4x
```

### Windows
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Release\aurora4x.exe
```

## Quick Start Tutorial

### 1. Starting Your Empire
When you start the game, you'll be prompted for your empire name. Press Enter to use the default "Earth Empire" or type your own name.

### 2. First Turn Strategy
On your first turns, focus on:
- **Research**: Start researching basic technologies
- **Exploration**: Explore nearby star systems
- **Resources**: Monitor your resource production

### 3. Research Path

#### Beginner Path (Recommended)
1. **Basic Mining** (100 RP) - Improves resource production
2. **Nuclear Power** (150 RP) - Powers advanced systems
3. **Ion Drive** (200 RP) - Enables better propulsion
4. **Warp Theory** (500 RP) - Unlocks FTL travel
5. **Warp Drive I** (1000 RP) - First interstellar drive

#### Military Path
1. **Missile Technology** (150 RP)
2. **Laser Weapons** (700 RP)
3. **Fusion Power** (600 RP)
4. **Basic Shields** (800 RP)
5. **Plasma Weapons** (1800 RP)

### 4. Sample Play Session

```
Turn 1-5: Research Basic Technologies
- Advance turns to accumulate research points
- Start with "Basic Mining" or "Nuclear Power"
- Watch your resources grow

Turn 6-15: Explore the Galaxy
- Use the Explore menu to discover new star systems
- Look for planets with good mineral deposits
- Identify colonization targets

Turn 16-30: Build Your Fleet
- Add more ships to your starting fleet
- Build Scouts for exploration
- Build Corvettes and Frigates for defense

Turn 31+: Advanced Game
- Research advanced technologies
- Expand to multiple colonies
- Test your fleets in combat simulations
```

## Menu Guide

### 1. Empire Status
Shows:
- Current turn number
- Number of colonies and fleets
- Researched technologies
- Resource levels (Minerals, Energy, Research Points)

**When to use**: Check at the start of each session to see your progress

### 2. Research
- View available technologies
- See prerequisites and costs
- Start researching new tech
- Monitor research progress

**Tips**: 
- Research points are generated automatically each turn (5 per turn base)
- Technologies with unmet prerequisites won't appear
- Higher tier techs require more research points

### 3. Explore Galaxy
- View explored and unexplored star systems
- Explore new systems to find planets
- Check planet types and mineral deposits

**Tips**:
- Terrestrial and Ocean planets are colonizable
- Each planet has different mineral deposits
- Systems are 3D positioned (x, y, z coordinates)

### 4. Fleet Management
- View all your fleets
- See ship details (hull, shields, weapons)
- Build new ships (Scout, Corvette, Frigate)
- Check combat strength

**Ship Classes**:
- **Scout**: Fast, weak, 1 laser (Hull: 50, Shields: 20)
- **Corvette**: Balanced, 2 lasers (Hull: 100, Shields: 50)
- **Frigate**: Strong, 3 lasers (Hull: 200, Shields: 100)

### 5. Combat Simulation
- Test battles between your fleets
- See detailed combat logs
- Understand combat mechanics

**Combat Mechanics**:
- Combat resolves in rounds
- Each ship fires at random targets
- Shields absorb damage first
- Destroyed ships are removed
- Victory goes to the last fleet standing

### 6. Advance Turn
- Progress the game one turn
- Generate resources
- Advance research
- Grow colonies

**Each turn produces**:
- Minerals: +10
- Energy: +50
- Research Points: +5

### 7. Help
Shows in-game help text

## Advanced Tips

### Resource Management
- Save up research points before starting expensive research
- Minerals and Energy aren't used yet but will be in future updates
- Population grows slowly on colonies

### Technology Strategy
1. **Economic Focus**: Basic Mining → Advanced Mining → Orbital Construction
2. **Military Focus**: Weapons → Shields → Better Weapons
3. **Exploration Focus**: Propulsion → Warp Drives → Sensors

### Fleet Building
- Scouts are cheap but die quickly
- Mix ship types for versatility
- Larger fleets have better odds in combat
- Combat is somewhat random due to weapon accuracy

### Exploration Tips
- The Sol system is your home (always explored)
- Other systems are randomly generated
- Planet types affect colonization potential
- Mineral deposits vary widely

## Technical Information

### Game Statistics
- **Technology Eras**: 5 (Pre-Warp to Future)
- **Total Technologies**: 25+
- **Ship Classes**: 8 types (3 buildable currently)
- **Star Systems**: 20 (configurable)
- **Planet Types**: 6 varieties
- **Mineral Types**: 10 (Aurora 4X style)

### Save/Load
Currently not implemented - each game session is independent

### Performance
- Fast turn processing
- Instant combat resolution
- Low memory footprint
- No graphics required

## Troubleshooting

### Build Issues
**Error: CMake not found**
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# Mac
brew install cmake
```

**Error: C++ compiler not found**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# Mac
xcode-select --install
```

### Game Issues
**Research not progressing**
- Make sure you've selected a technology (option 2 → R <number>)
- Advance turns to generate research points
- Check that prerequisites are met

**Can't build ships**
- Make sure fleet name is spelled correctly
- Ship class must be: Scout, Corvette, or Frigate
- Use format: "B Scout Home Defense Fleet"

## Next Steps

Once you're comfortable with the basics:
1. Try to research all Pre-Warp technologies
2. Achieve Warp Drive I for interstellar travel
3. Build a fleet of 10+ ships
4. Explore all 20 star systems
5. Research into Advanced or Future eras

## Future Features (Planned)

- Save/Load game functionality
- More ship classes and weapons
- AI empires and diplomacy
- Colony management and construction
- Ship design system
- Trade and economy
- Random events
- Victory conditions

Enjoy building your space empire!
