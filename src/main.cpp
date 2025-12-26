#include "game.h"
#include "ui.h"
#include <iostream>
#include <string>
#include <sstream>

// Forward declarations
void displayEmpireStatus(Game& game, UIManager& ui);
void researchMenu(Game& game, UIManager& ui);
void exploreMenu(Game& game, UIManager& ui);
void fleetMenu(Game& game, UIManager& ui);
void combatMenu(Game& game, UIManager& ui);
void showHelp(UIManager& ui);

void displayEmpireStatus(Game& game, UIManager& ui) {
    auto empire = game.getEmpire();
    
    std::ostringstream oss;
    oss << "EMPIRE: " << empire->getName() << "\n\n";
    oss << "Turn: " << empire->getTurn() << "\n";
    oss << "Colonies: " << empire->getColonies().size() << "\n";
    oss << "Fleets: " << empire->getFleets().size() << "\n";
    oss << "Researched Technologies: " << empire->getResearch().getResearchedCount() << "\n";
    if (empire->getCurrentResearch().empty()) {
        oss << "Current Research: None\n";
    } else {
        auto tech = empire->getResearch().getTech(empire->getCurrentResearch());
        if (tech) {
            oss << "Current Research: " << tech->getName()
                << " (" << tech->getProgress() << "/" << tech->getCost() << ")\n";
        } else {
            oss << "Current Research: " << empire->getCurrentResearch() << "\n";
        }
    }
    oss << "\nResources:\n";
    oss << "  Minerals: " << empire->getResources().get(ResourceType::MINERALS) << "\n";
    oss << "  Energy: " << empire->getResources().get(ResourceType::ENERGY) << "\n";
    oss << "  Research Points: " << empire->getResources().get(ResourceType::RESEARCH_POINTS) << "\n";
    
    ui.displayText(oss.str(), true);
}

void researchMenu(Game& game, UIManager& ui) {
    bool inResearchMenu = true;
    
    while (inResearchMenu) {
        auto empire = game.getEmpire();
        auto available = game.getAvailableResearch();
        
        std::vector<MenuItem> researchItems;
        
        for (size_t i = 0; i < available.size() && i < 10; ++i) {
            auto tech = available[i];
            std::ostringstream label;
            label << tech->getName() << " (" << techCategoryToString(tech->getCategory())
                  << ", Cost: " << tech->getCost() << " RP)";
            
            researchItems.push_back(MenuItem(label.str(), [&game, tech, &ui]() {
                std::string result = game.startResearch(tech->getId());
                ui.displayText(result, true);
            }));
        }
        
        researchItems.push_back(MenuItem("Back to Main Menu", [&inResearchMenu]() {
            inResearchMenu = false;
        }));
        
        std::ostringstream title;
        title << "RESEARCH MENU (RP: " << empire->getResources().get(ResourceType::RESEARCH_POINTS);
        if (!empire->getCurrentResearch().empty()) {
            auto current = empire->getResearch().getTech(empire->getCurrentResearch());
            if (current) {
                title << ", Current: " << current->getName() << " "
                      << current->getProgress() << "/" << current->getCost();
            }
        }
        title << ")";
        
        int choice = ui.displayMenu(title.str(), researchItems);
        
        if (choice >= 0 && choice < static_cast<int>(researchItems.size())) {
            researchItems[choice].action();
        } else {
            inResearchMenu = false;
        }
    }
}

void exploreMenu(Game& game, UIManager& ui) {
    auto galaxy = game.getGalaxy();
    auto unexplored = galaxy->getUnexploredSystems();
    
    std::ostringstream info;
    info << "Explored Systems: " << galaxy->getExploredSystems().size() << "\n";
    info << "Unexplored Systems: " << unexplored.size() << "\n\n";
    
    std::vector<MenuItem> exploreItems;
    
    size_t showCount = std::min(unexplored.size(), size_t(10));
    for (size_t i = 0; i < showCount; ++i) {
        auto system = unexplored[i];
        std::ostringstream label;
        label << "Explore " << system->getName() << " (" << system->getX() << "," 
              << system->getY() << "," << system->getZ() << ")";
        
        exploreItems.push_back(MenuItem(label.str(), [&game, system, &ui]() {
            std::string result = game.exploreSystem(system->getName());
            std::ostringstream details;
            details << result << "\n\nPlanets found:\n";
            for (const auto& planet : system->getPlanets()) {
                details << "  " << planet->getName() << " (" << planet->getPlanetType() << ")\n";
            }
            ui.displayText(details.str(), true);
        }));
    }
    
    exploreItems.push_back(MenuItem("Back to Main Menu", []() {}));
    
    int choice = ui.displayMenu("GALAXY EXPLORATION", exploreItems);
    
    if (choice >= 0 && choice < static_cast<int>(exploreItems.size())) {
        exploreItems[choice].action();
    }
}

void fleetMenu(Game& game, UIManager& ui) {
    auto fleets = game.getEmpire()->getFleets();
    
    std::ostringstream info;
    info << "Your Fleets:\n\n";
    
    for (size_t i = 0; i < fleets.size(); ++i) {
        auto fleet = fleets[i];
        info << (i + 1) << ". " << fleet->getName() << "\n";
        info << "   Location: " << (fleet->getLocation() ? fleet->getLocation()->getName() : "Unknown") << "\n";
        info << "   Ships: " << fleet->getShips().size() << "\n";
        info << "   Combat Strength: " << fleet->getCombatStrength() << "\n";
    }
    
    std::vector<MenuItem> fleetItems = {
        MenuItem("Build Fighter", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::FIGHTER, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Scout", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::SCOUT, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Corvette", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::CORVETTE, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Frigate", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::FRIGATE, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Destroyer", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::DESTROYER, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Cruiser", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::CRUISER, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Battleship", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::BATTLESHIP, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Build Carrier", [&game, &ui]() {
            std::string fleetName = ui.getInput("Enter fleet name: ");
            if (!fleetName.empty()) {
                std::string result = game.buildShip(ShipClass::CARRIER, fleetName);
                ui.displayText(result, true);
            }
        }),
        MenuItem("Back to Main Menu", []() {})
    };
    
    ui.displayText(info.str(), true);
    
    int choice = ui.displayMenu("FLEET MANAGEMENT", fleetItems);
    
    if (choice >= 0 && choice < static_cast<int>(fleetItems.size())) {
        fleetItems[choice].action();
    }
}

void combatMenu(Game& game, UIManager& ui) {
    auto fleets = game.getEmpire()->getFleets();
    
    if (fleets.size() < 2) {
        ui.displayText("Need at least 2 fleets for combat simulation", true);
        return;
    }
    
    std::ostringstream info;
    info << "Available Fleets:\n\n";
    for (size_t i = 0; i < fleets.size(); ++i) {
        info << (i + 1) << ". " << fleets[i]->getName() 
             << " (Strength: " << fleets[i]->getCombatStrength() << ")\n";
    }
    info << "\nSelect two fleets for combat simulation:";
    
    ui.displayText(info.str(), true);
    
    int fleet1 = ui.getIntInput("Enter first fleet number: ", 0);
    int fleet2 = ui.getIntInput("Enter second fleet number: ", 0);
    
    if (fleet1 > 0 && fleet1 <= static_cast<int>(fleets.size()) &&
        fleet2 > 0 && fleet2 <= static_cast<int>(fleets.size()) &&
        fleet1 != fleet2) {
        
        std::string result = game.simulateCombat(
            fleets[fleet1 - 1]->getName(),
            fleets[fleet2 - 1]->getName()
        );
        ui.displayText(result, true);
    } else {
        ui.displayText("Invalid fleet selection", true);
    }
}

void showHelp(UIManager& ui) {
    std::string helpText = R"(
AURORA 4X-LIKE - Help

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

RESOURCES:
- Minerals: Used for construction
- Energy: Powers systems
- Research Points: Generated each turn for research

COMBAT:
Ships have hull and shields. Weapons have damage and accuracy.
Combat is resolved in rounds until one side is defeated.

MOUSE CONTROLS:
- Click on menu items to select them
- Use arrow keys if mouse is not available
- Press ESC to go back
)";
    
    ui.displayText(helpText, true);
}

int main() {
    UIManager ui;
    ui.init();
    
    // Display welcome screen
    std::ostringstream welcome;
    welcome << "AURORA 4X-LIKE: Space Empire Builder\n\n";
    if (ui.hasMouseSupport()) {
        welcome << "Mouse support enabled!\n";
        welcome << "Click menu items to select them or use arrow keys + Enter.";
    } else {
        welcome << "Use arrow keys and Enter to navigate menus.";
    }
    ui.displayText(welcome.str(), true);
    
    // Get empire name
    std::string empireName = ui.getInput("Enter your empire name (or press Enter for 'Earth Empire'): ");
    if (empireName.empty()) {
        empireName = "Earth Empire";
    }
    
    // Create game
    Game game(empireName);
    
    std::ostringstream intro;
    intro << "Welcome, leader of " << empireName << "!\n\n"
          << "Your civilization has just achieved spaceflight capability.\n"
          << "Guide your empire from pre-warp to the stars!";
    ui.displayText(intro.str(), true);
    
    // Main game loop with mouse-driven menu
    bool running = true;
    while (running) {
        std::vector<MenuItem> mainMenu = {
            MenuItem("Empire Status", [&]() { displayEmpireStatus(game, ui); }),
            MenuItem("Research", [&]() { researchMenu(game, ui); }),
            MenuItem("Explore Galaxy", [&]() { exploreMenu(game, ui); }),
            MenuItem("Fleet Management", [&]() { fleetMenu(game, ui); }),
            MenuItem("Combat Simulation", [&]() { combatMenu(game, ui); }),
            MenuItem("Advance Turn", [&]() { 
                std::string result = game.advanceTurn();
                auto empire = game.getEmpire();
                std::ostringstream msg;
                msg << result << "\n\n"
                    << "Resources:\n"
                    << "  Minerals: " << empire->getResources().get(ResourceType::MINERALS) << "\n"
                    << "  Energy: " << empire->getResources().get(ResourceType::ENERGY) << "\n"
                    << "  Research: " << empire->getResources().get(ResourceType::RESEARCH_POINTS) << "\n";

                if (empire->getCurrentResearch().empty()) {
                    msg << "Current Research: None";
                } else {
                    auto tech = empire->getResearch().getTech(empire->getCurrentResearch());
                    if (tech) {
                        msg << "Current Research: " << tech->getName()
                            << " (" << tech->getProgress() << "/" << tech->getCost() << ")";
                    } else {
                        msg << "Current Research: " << empire->getCurrentResearch();
                    }
                }
                ui.displayText(msg.str(), true);
            }),
            MenuItem("Help", [&]() { showHelp(ui); }),
            MenuItem("Exit", [&]() { running = false; })
        };
        
        int choice = ui.displayMenu("MAIN MENU", mainMenu);
        
        if (choice >= 0 && choice < static_cast<int>(mainMenu.size())) {
            mainMenu[choice].action();
        } else if (choice == -1) {
            // User pressed ESC or back - treat as exit
            running = false;
        }
    }
    
    ui.cleanup();
    std::cout << "\nThank you for playing!\n";
    
    return 0;
}
