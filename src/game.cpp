#include "game.h"
#include "battle_viewer.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <random>
#include <sstream>

namespace {
static Weapon makeHeavyLaser() { return Weapon("Heavy Laser", 15, 0.75, 6); }
static Weapon makeRailgun() { return Weapon("Railgun", 20, 0.65, 6); }

static int fleetTotalHP(const std::shared_ptr<Fleet>& f) {
    if (!f) return 0;
    int total = 0;
    for (const auto& ship : f->getShips()) {
        if (!ship) continue;
        total += std::max(0, ship->getHull()) + std::max(0, ship->getShields());
    }
    return total;
}

static int fleetShipCount(const std::shared_ptr<Fleet>& f) {
    if (!f) return 0;
    int count = 0;
    for (const auto& ship : f->getShips()) {
        if (ship && ship->isOperational()) ++count;
    }
    return count;
}

static Weapon makeBestBeam(const Empire& e) {
    // Simple progression.
    const auto& r = e.getResearch();
    if (r.isResearched("singularity_weapons")) return Weapon("Singularity", 35, 0.6, 9);
    if (r.isResearched("particle_beam")) return Weapon("Particle Beam", 24, 0.7, 7);
    if (r.isResearched("plasma_weapons")) return Weapon("Plasma", 18, 0.7, 6);
    if (r.isResearched("laser_weapons")) return Weapon("Laser", 12, 0.72, 5);
    return Weapon("Laser", 10, 0.7, 5);
}

static Weapon makeBestMissile(const Empire& e) {
    const auto& r = e.getResearch();
    double acc = 0.55;
    if (r.isResearched("missile_guidance")) acc = 0.65;
    return Weapon("Missile", 25, acc, 8);
}

static void applyDefensesFromTech(const Empire& e, int& hull, int& shields) {
    const auto& r = e.getResearch();
    if (r.isResearched("reinforced_hulls")) hull = (hull * 110) / 100;
    if (r.isResearched("nanomaterials")) hull = (hull * 115) / 100;
    if (r.isResearched("self_repairing_hulls")) hull = (hull * 120) / 100;

    if (r.isResearched("basic_shields")) shields = (shields * 110) / 100;
    if (r.isResearched("advanced_shields")) shields = (shields * 120) / 100;
    if (r.isResearched("graviton_shields")) shields = (shields * 130) / 100;
    if (r.isResearched("phase_shields")) shields = (shields * 140) / 100;
    if (r.isResearched("void_shields")) shields = (shields * 150) / 100;
}

static std::shared_ptr<Ship> makeShipForClass(const Empire& e, const std::string& baseName, ShipClass shipClass, int index) {
    const std::string shipName = baseName + "-" + shipClassToString(shipClass) + "-" + std::to_string(index);

    Weapon beam = makeBestBeam(e);
    Weapon missile = makeBestMissile(e);
    Weapon railgun = makeRailgun();
    Weapon heavy = makeHeavyLaser();

    auto shipWith = [&](int hull, int shields, std::vector<Weapon> wpns) {
        applyDefensesFromTech(e, hull, shields);
        return std::make_shared<Ship>(shipName, shipClass, hull, shields, wpns);
    };

    switch (shipClass) {
        case ShipClass::FIGHTER:
            return shipWith(30, 10, std::vector<Weapon>{beam});
        case ShipClass::SCOUT:
            return shipWith(50, 20, std::vector<Weapon>{beam});
        case ShipClass::CORVETTE:
            return shipWith(100, 50, std::vector<Weapon>{beam, beam});
        case ShipClass::FRIGATE:
            return shipWith(200, 100, std::vector<Weapon>{heavy, heavy, beam});
        case ShipClass::DESTROYER:
            return shipWith(300, 140, std::vector<Weapon>{heavy, beam, railgun, railgun});
        case ShipClass::CRUISER:
            return shipWith(500, 250, std::vector<Weapon>{heavy, beam, railgun, railgun, missile});
        case ShipClass::BATTLESHIP:
            return shipWith(900, 450, std::vector<Weapon>{heavy, heavy, beam, railgun, railgun, missile, missile});
        case ShipClass::CARRIER:
            return shipWith(700, 350, std::vector<Weapon>{beam, beam, railgun, missile});
        default:
            return shipWith(50, 20, std::vector<Weapon>{beam});
    }
}

static bool fleetHasOperationalShips(const std::shared_ptr<Fleet>& fleet) {
    if (!fleet) return false;
    for (const auto& ship : fleet->getShips()) {
        if (ship && ship->isOperational()) return true;
    }
    return false;
}

static std::shared_ptr<Fleet> pickRandomOperationalFleet(const std::vector<std::shared_ptr<Fleet>>& fleets, std::mt19937& gen) {
    std::vector<std::shared_ptr<Fleet>> candidates;
    for (const auto& f : fleets) {
        if (fleetHasOperationalShips(f)) candidates.push_back(f);
    }
    if (candidates.empty()) return nullptr;
    std::uniform_int_distribution<std::size_t> dis(0, candidates.size() - 1);
    return candidates[dis(gen)];
}

static ShipClass aiPickBuildClass(int turn, std::mt19937& gen) {
    // Slowly ramps up ship sizes over time.
    std::vector<ShipClass> options;
    options.push_back(ShipClass::FIGHTER);
    options.push_back(ShipClass::SCOUT);
    options.push_back(ShipClass::CORVETTE);
    if (turn >= 4) options.push_back(ShipClass::FRIGATE);
    if (turn >= 7) options.push_back(ShipClass::DESTROYER);
    if (turn >= 10) options.push_back(ShipClass::CRUISER);
    if (turn >= 14) options.push_back(ShipClass::BATTLESHIP);
    if (turn >= 16) options.push_back(ShipClass::CARRIER);

    std::uniform_int_distribution<std::size_t> dis(0, options.size() - 1);
    return options[dis(gen)];
}

static std::string trim(std::string s) {
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && isSpace((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && isSpace((unsigned char)s.back())) s.pop_back();
    return s;
}

static bool parseInt(const std::string& s, int& out) {
    try {
        size_t idx = 0;
        const int v = std::stoi(s, &idx);
        if (idx != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

static std::string findSystemForPlanet(const Galaxy& g, const std::shared_ptr<Planet>& planet) {
    if (!planet) return {};
    for (const auto& sys : g.getSystems()) {
        if (!sys) continue;
        for (const auto& p : sys->getPlanets()) {
            if (p == planet) return sys->getName();
        }
    }
    return {};
}

static std::shared_ptr<Planet> findPlanetInSystem(const std::shared_ptr<StarSystem>& sys, const std::string& planetName) {
    if (!sys) return nullptr;
    for (const auto& p : sys->getPlanets()) {
        if (p && p->getName() == planetName) return p;
    }
    return nullptr;
}

static std::string serializeResources(const ResourceStorage& r) {
    // Comma-separated key:value pairs.
    std::ostringstream oss;
    bool first = true;
    for (const auto& kv : r.snapshot()) {
        if (!first) oss << ",";
        first = false;
        oss << resourceTypeToString(kv.first) << ":" << kv.second;
    }
    return oss.str();
}

static void applyResourcesForLoad(ResourceStorage& r, const std::string& encoded) {
    for (const auto& item : split(encoded, ',')) {
        const auto kv = split(trim(item), ':');
        if (kv.size() != 2) continue;
        ResourceType t;
        int amount = 0;
        if (!resourceTypeFromString(trim(kv[0]), t)) continue;
        if (!parseInt(trim(kv[1]), amount)) continue;
        r.set(t, amount);
    }
}

static std::shared_ptr<Ship> makeNamedShipForClass(const Empire& e, const std::string& shipName, ShipClass shipClass) {
    Weapon beam = makeBestBeam(e);
    Weapon missile = makeBestMissile(e);
    Weapon railgun = makeRailgun();
    Weapon heavy = makeHeavyLaser();

    auto shipWith = [&](int hull, int shields, std::vector<Weapon> wpns) {
        applyDefensesFromTech(e, hull, shields);
        return std::make_shared<Ship>(shipName, shipClass, hull, shields, wpns);
    };

    switch (shipClass) {
        case ShipClass::FIGHTER:
            return shipWith(30, 10, std::vector<Weapon>{beam});
        case ShipClass::SCOUT:
            return shipWith(50, 20, std::vector<Weapon>{beam});
        case ShipClass::CORVETTE:
            return shipWith(100, 50, std::vector<Weapon>{beam, beam});
        case ShipClass::FRIGATE:
            return shipWith(200, 100, std::vector<Weapon>{heavy, heavy, beam});
        case ShipClass::DESTROYER:
            return shipWith(300, 140, std::vector<Weapon>{heavy, beam, railgun, railgun});
        case ShipClass::CRUISER:
            return shipWith(500, 250, std::vector<Weapon>{heavy, beam, railgun, railgun, missile});
        case ShipClass::BATTLESHIP:
            return shipWith(900, 450, std::vector<Weapon>{heavy, heavy, beam, railgun, railgun, missile, missile});
        case ShipClass::CARRIER:
            return shipWith(700, 350, std::vector<Weapon>{beam, beam, railgun, missile});
        default:
            return shipWith(50, 20, std::vector<Weapon>{beam});
    }
}
} // namespace

Game::Game(const std::string& empireName, uint32_t galaxySeed)
    : empire(std::make_shared<Empire>(empireName)),
      galaxy(std::make_shared<Galaxy>(20, galaxySeed)),
      running(false) {
    setupGame();
}

std::string Game::quickSave(const std::string& path) const {
    if (!empire || !galaxy) return "Cannot save: game not initialized";

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return "Cannot save: failed to open file: " + path;
    }

    out << "AURORA_SAVE_V1\n";
    out << "seed=" << galaxy->getSeed() << "\n";
    out << "numSystems=" << galaxy->getSystems().size() << "\n";

    out << "[Player]\n";
    out << "name=" << empire->getName() << "\n";
    out << "turn=" << empire->getTurn() << "\n";
    out << "currentResearch=" << empire->getCurrentResearch() << "\n";
    out << "resources=" << serializeResources(empire->getResources()) << "\n";

    for (const auto& tech : empire->getResearch().getAllTechs()) {
        if (!tech) continue;
        if (tech->isResearched() || tech->getProgress() > 0) {
            out << "tech=" << tech->getId() << "," << tech->getProgress() << "," << (tech->isResearched() ? 1 : 0) << "\n";
        }
    }

    out << "[Explored]\n";
    for (const auto& sys : galaxy->getExploredSystems()) {
        if (!sys) continue;
        out << "system=" << sys->getName() << "\n";
    }

    out << "[Colonies]\n";
    for (const auto& c : empire->getColonies()) {
        if (!c) continue;
        const std::string sysName = findSystemForPlanet(*galaxy, c->getPlanet());
        const std::string planetName = c->getPlanet() ? c->getPlanet()->getName() : "";
        out << "colony=" << c->getName()
            << ";system=" << sysName
            << ";planet=" << planetName
            << ";pop=" << c->getPopulation()
            << ";mines=" << c->getMines()
            << ";factories=" << c->getFactories() << "\n";
    }

    out << "[Fleets]\n";
    for (const auto& f : empire->getFleets()) {
        if (!f) continue;
        const std::string sysName = f->getLocation() ? f->getLocation()->getName() : "";
        out << "fleet=" << f->getName() << ";system=" << sysName << "\n";
        for (const auto& ship : f->getShips()) {
            if (!ship) continue;
            out << "ship=" << ship->getName()
                << ";class=" << shipClassToString(ship->getShipClass())
                << ";hull=" << ship->getHull()
                << ";shields=" << ship->getShields() << "\n";
        }
        out << "endfleet\n";
    }

    out << "[Hostiles]\n";
    for (const auto& h : hostileEmpires) {
        if (!h) continue;
        out << "[Hostile]\n";
        out << "name=" << h->getName() << "\n";
        out << "contacted=" << (isHostileContacted(h->getName()) ? 1 : 0) << "\n";
        out << "atWar=" << (isHostileAtWar(h->getName()) ? 1 : 0) << "\n";
        out << "turn=" << h->getTurn() << "\n";
        out << "currentResearch=" << h->getCurrentResearch() << "\n";
        out << "resources=" << serializeResources(h->getResources()) << "\n";
        for (const auto& tech : h->getResearch().getAllTechs()) {
            if (!tech) continue;
            if (tech->isResearched() || tech->getProgress() > 0) {
                out << "tech=" << tech->getId() << "," << tech->getProgress() << "," << (tech->isResearched() ? 1 : 0) << "\n";
            }
        }
        for (const auto& c : h->getColonies()) {
            if (!c) continue;
            const std::string sysName = findSystemForPlanet(*galaxy, c->getPlanet());
            const std::string planetName = c->getPlanet() ? c->getPlanet()->getName() : "";
            out << "colony=" << c->getName()
                << ";system=" << sysName
                << ";planet=" << planetName
                << ";pop=" << c->getPopulation()
                << ";mines=" << c->getMines()
                << ";factories=" << c->getFactories() << "\n";
        }
        for (const auto& f : h->getFleets()) {
            if (!f) continue;
            const std::string sysName = f->getLocation() ? f->getLocation()->getName() : "";
            out << "fleet=" << f->getName() << ";system=" << sysName << "\n";
            for (const auto& ship : f->getShips()) {
                if (!ship) continue;
                out << "ship=" << ship->getName()
                    << ";class=" << shipClassToString(ship->getShipClass())
                    << ";hull=" << ship->getHull()
                    << ";shields=" << ship->getShields() << "\n";
            }
            out << "endfleet\n";
        }
        out << "endhostile\n";
    }

    return "Saved to " + path;
}

std::string Game::quickLoad(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return "Cannot load: failed to open file: " + path;
    }

    std::string header;
    std::getline(in, header);
    header = trim(header);
    if (header != "AURORA_SAVE_V1") {
        return "Cannot load: invalid save header";
    }

    struct SavedShip { std::string name; ShipClass cls{ShipClass::SCOUT}; int hull{0}; int shields{0}; };
    struct SavedFleet { std::string name; std::string system; std::vector<SavedShip> ships; };
    struct SavedColony { std::string name; std::string system; std::string planet; int pop{10}; int mines{0}; int factories{0}; };
    struct SavedTech { std::string id; int progress{0}; bool researched{false}; };
    struct SavedEmpire {
        std::string name;
        int turn{0};
        std::string currentResearch;
        std::string resources;
        std::vector<SavedTech> techs;
        std::vector<SavedColony> colonies;
        std::vector<SavedFleet> fleets;
    };
    struct SavedHostile {
        SavedEmpire e;
        bool contacted{false};
        bool atWar{false};
    };

    uint32_t seed = 0;
    int numSystems = 20;
    std::vector<std::string> exploredSystems;
    SavedEmpire player;
    std::vector<SavedHostile> hostiles;

    enum class Section { None, Player, Explored, Colonies, Fleets, Hostiles, Hostile };
    Section section = Section::None;
    SavedHostile* curHostile = nullptr;
    SavedFleet* curFleet = nullptr;

    auto& curEmpire = [&]() -> SavedEmpire& {
        if (section == Section::Hostile && curHostile) return curHostile->e;
        return player;
    };

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
            const std::string tag = line.substr(1, line.size() - 2);
            if (tag == "Player") { section = Section::Player; curHostile = nullptr; curFleet = nullptr; }
            else if (tag == "Explored") { section = Section::Explored; curHostile = nullptr; curFleet = nullptr; }
            else if (tag == "Colonies") { section = Section::Colonies; curHostile = nullptr; curFleet = nullptr; }
            else if (tag == "Fleets") { section = Section::Fleets; curHostile = nullptr; curFleet = nullptr; }
            else if (tag == "Hostiles") { section = Section::Hostiles; curHostile = nullptr; curFleet = nullptr; }
            else if (tag == "Hostile") {
                section = Section::Hostile;
                hostiles.push_back(SavedHostile{});
                curHostile = &hostiles.back();
                curFleet = nullptr;
            }
            continue;
        }

        const auto kv = split(line, '=');
        if (kv.size() < 2) {
            if (line == "endfleet") {
                curFleet = nullptr;
                continue;
            }
            if (line == "endhostile") {
                curHostile = nullptr;
                section = Section::Hostiles;
                curFleet = nullptr;
                continue;
            }
            continue;
        }

        const std::string key = trim(kv[0]);
        const std::string value = trim(line.substr(line.find('=') + 1));

        if (section == Section::None) {
            if (key == "seed") {
                int tmp = 0;
                if (parseInt(value, tmp)) seed = static_cast<uint32_t>(tmp);
            } else if (key == "numSystems") {
                int tmp = 0;
                if (parseInt(value, tmp)) numSystems = tmp;
            }
            continue;
        }

        if (section == Section::Explored) {
            if (key == "system") exploredSystems.push_back(value);
            continue;
        }

        if (section == Section::Player || section == Section::Hostile) {
            auto& e = curEmpire();
            if (key == "name") e.name = value;
            else if (key == "turn") { int t = 0; if (parseInt(value, t)) e.turn = t; }
            else if (key == "currentResearch") e.currentResearch = value;
            else if (key == "resources") e.resources = value;
            else if (key == "contacted" && curHostile) { int v = 0; if (parseInt(value, v)) curHostile->contacted = (v != 0); }
            else if (key == "atWar" && curHostile) { int v = 0; if (parseInt(value, v)) curHostile->atWar = (v != 0); }
            else if (key == "tech") {
                const auto parts = split(value, ',');
                if (parts.size() >= 3) {
                    SavedTech t;
                    t.id = trim(parts[0]);
                    parseInt(trim(parts[1]), t.progress);
                    int rf = 0;
                    parseInt(trim(parts[2]), rf);
                    t.researched = (rf != 0);
                    e.techs.push_back(std::move(t));
                }
            } else if (key == "colony") {
                SavedColony c;
                // Parse semicolon-delimited k=v pairs with the first token being the name.
                const auto toks = split(value, ';');
                c.name = trim(toks[0]);
                for (size_t i = 1; i < toks.size(); ++i) {
                    const auto kv2 = split(toks[i], '=');
                    if (kv2.size() != 2) continue;
                    const std::string k2 = trim(kv2[0]);
                    const std::string v2 = trim(kv2[1]);
                    if (k2 == "system") c.system = v2;
                    else if (k2 == "planet") c.planet = v2;
                    else if (k2 == "pop") parseInt(v2, c.pop);
                    else if (k2 == "mines") parseInt(v2, c.mines);
                    else if (k2 == "factories") parseInt(v2, c.factories);
                }
                e.colonies.push_back(std::move(c));
            } else if (key == "fleet") {
                SavedFleet f;
                const auto toks = split(value, ';');
                f.name = trim(toks[0]);
                for (size_t i = 1; i < toks.size(); ++i) {
                    const auto kv2 = split(toks[i], '=');
                    if (kv2.size() != 2) continue;
                    if (trim(kv2[0]) == "system") f.system = trim(kv2[1]);
                }
                e.fleets.push_back(std::move(f));
                curFleet = &e.fleets.back();
            } else if (key == "ship" && curFleet) {
                SavedShip sship;
                const auto toks = split(value, ';');
                sship.name = trim(toks[0]);
                for (size_t i = 1; i < toks.size(); ++i) {
                    const auto kv2 = split(toks[i], '=');
                    if (kv2.size() != 2) continue;
                    const std::string k2 = trim(kv2[0]);
                    const std::string v2 = trim(kv2[1]);
                    if (k2 == "class") {
                        ShipClass sc;
                        if (shipClassFromString(v2, sc)) sship.cls = sc;
                    } else if (k2 == "hull") {
                        parseInt(v2, sship.hull);
                    } else if (k2 == "shields") {
                        parseInt(v2, sship.shields);
                    }
                }
                curFleet->ships.push_back(std::move(sship));
            }
            continue;
        }
    }

    // Construct fresh world from seed.
    auto newGalaxy = std::make_shared<Galaxy>(numSystems, seed);
    for (const auto& sysName : exploredSystems) {
        if (auto sys = newGalaxy->findSystemByName(sysName)) sys->explore();
    }

    auto buildEmpireFromSaved = [&](const SavedEmpire& se, const std::string& ownerName) -> std::shared_ptr<Empire> {
        auto e = std::make_shared<Empire>(ownerName);
        e->setTurnForLoad(se.turn);
        applyResourcesForLoad(e->getResources(), se.resources);
        for (const auto& t : se.techs) {
            e->getResearch().setTechStateForLoad(t.id, t.progress, t.researched);
        }

        if (!se.currentResearch.empty()) {
            if (!e->setResearch(se.currentResearch)) {
                e->setCurrentResearchForLoad(se.currentResearch);
            }
        }

        // Colonies
        for (const auto& c : se.colonies) {
            auto sys = newGalaxy->findSystemByName(c.system);
            auto planet = findPlanetInSystem(sys, c.planet);
            if (!planet) continue;
            auto colony = std::make_shared<Colony>(c.name, planet);
            colony->setPopulationForLoad(c.pop);
            colony->setMinesForLoad(c.mines);
            colony->setFactoriesForLoad(c.factories);
            planet->colonize(colony);
            e->addColony(colony);
        }

        // Fleets
        for (const auto& f : se.fleets) {
            auto fleet = std::make_shared<Fleet>(f.name, ownerName);
            if (!f.system.empty()) {
                fleet->setLocation(newGalaxy->findSystemByName(f.system));
            }
            for (const auto& sh : f.ships) {
                auto ship = makeNamedShipForClass(*e, sh.name, sh.cls);
                if (ship) {
                    const int maxH = ship->getMaxHull();
                    const int maxS = ship->getMaxShields();
                    const int wantH = std::max(0, std::min(sh.hull, maxH));
                    const int wantS = std::max(0, std::min(sh.shields, maxS));

                    if (wantH == maxH) {
                        const int dmg = maxS - wantS;
                        if (dmg > 0) ship->takeDamage(dmg);
                    } else {
                        // Hull damage implies shields were depleted at some point.
                        const int dmg = maxS + (maxH - wantH);
                        if (dmg > 0) ship->takeDamage(dmg);
                    }
                    fleet->addShip(ship);
                }
            }
            e->addFleet(fleet);
        }

        return e;
    };

    if (player.name.empty()) player.name = "Earth Empire";
    auto newEmpire = buildEmpireFromSaved(player, player.name);

    std::vector<std::shared_ptr<Empire>> newHostiles;
    std::map<std::string, bool> newContacted;
    std::map<std::string, bool> newAtWar;

    for (const auto& h : hostiles) {
        const std::string name = h.e.name.empty() ? "Hostile" : h.e.name;
        auto e = buildEmpireFromSaved(h.e, name);
        newHostiles.push_back(e);
        newContacted[name] = h.contacted;
        newAtWar[name] = h.atWar;
    }

    empire = newEmpire;
    galaxy = newGalaxy;
    hostileEmpires = std::move(newHostiles);
    hostileContacted = std::move(newContacted);
    hostileAtWar = std::move(newAtWar);

    return "Loaded from " + path;
}

void Game::setupGame() {
    // Colonize home planet (Earth)
    auto homePlanets = galaxy->getHomeSystem()->getPlanets();
    if (homePlanets.size() >= 3) {
        auto homePlanet = homePlanets[2];  // 3rd planet
        auto earthColony = std::make_shared<Colony>("Earth", homePlanet);
        homePlanet->colonize(earthColony);
        empire->addColony(earthColony);
    }
    
    // Create starting fleet
    auto startingFleet = createStartingFleet();
    empire->addFleet(startingFleet);

    // Create hostile empires with their own starting fleets.
    // Minimal AI: they build up and sometimes attack on Advance Turn.
    const auto& systems = galaxy->getSystems();
    auto pickSystem = [&](std::size_t fallbackIndex) -> std::shared_ptr<StarSystem> {
        if (!systems.empty()) {
            const std::size_t idx = std::min(fallbackIndex, systems.size() - 1);
            return systems[idx];
        }
        return galaxy->getHomeSystem();
    };

    struct AiSpec { const char* name; std::size_t sysIndex; };
    const AiSpec specs[] = {
        {"Zorg Collective", 5},
        {"Krell Dominion", 8},
    };

    for (const auto& spec : specs) {
        auto ai = std::make_shared<Empire>(spec.name);
        auto fleet = std::make_shared<Fleet>(std::string(spec.name) + " Fleet", ai->getName());
        fleet->addShip(makeShipForClass(*ai, "Raider", ShipClass::CORVETTE, 1));
        fleet->addShip(makeShipForClass(*ai, "Raider", ShipClass::SCOUT, 2));
        fleet->setLocation(pickSystem(spec.sysIndex));
        ai->addFleet(fleet);
        hostileEmpires.push_back(ai);

        hostileContacted[ai->getName()] = false;
        hostileAtWar[ai->getName()] = false;

        // Give each hostile a starting colony on a colonizable planet in its system (if any).
        if (auto sys = fleet->getLocation()) {
            auto colonizable = sys->getColonizablePlanets();
            if (!colonizable.empty()) {
                auto planet = colonizable[0];
                auto colony = std::make_shared<Colony>(std::string(spec.name) + " Prime", planet);
                planet->colonize(colony);
                ai->addColony(colony);
            }
        }
    }
}

bool Game::isHostileContacted(const std::string& hostileName) const {
    auto it = hostileContacted.find(hostileName);
    return it != hostileContacted.end() ? it->second : false;
}

bool Game::isHostileAtWar(const std::string& hostileName) const {
    auto it = hostileAtWar.find(hostileName);
    return it != hostileAtWar.end() ? it->second : false;
}

std::shared_ptr<Fleet> Game::createStartingFleet() {
    auto fleet = std::make_shared<Fleet>("Home Defense Fleet", empire->getName());
    
    // Add basic ships
    Weapon laser("Laser Cannon", 10, 0.7, 5);
    
    auto scout = std::make_shared<Ship>("Scout-1", ShipClass::SCOUT, 50, 20,
                                       std::vector<Weapon>{laser});
    auto corvette = std::make_shared<Ship>("Corvette-1", ShipClass::CORVETTE, 100, 50,
                                          std::vector<Weapon>{laser, laser});
    
    fleet->addShip(scout);
    fleet->addShip(corvette);
    fleet->setLocation(galaxy->getHomeSystem());
    
    return fleet;
}

std::string Game::advanceTurn() {
    std::ostringstream log;
    log << empire->advanceTurn();

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> chance(0.0, 1.0);

    // Hostile empires take their turns.
    for (auto& ai : hostileEmpires) {
        if (!ai) continue;

        int builtShips = 0;
        int colonizedPlanets = 0;
        bool startedResearch = false;
        bool attacked = false;
        std::string startedResearchName;

        log << "\n";
        log << "[Hostile] " << ai->getName() << ": " << ai->advanceTurn();

        // If not researching anything, pick the first available tech.
        if (ai->getCurrentResearch().empty()) {
            auto available = ai->getResearch().getAvailableTechs();
            if (!available.empty() && available[0]) {
                ai->setResearch(available[0]->getId());
                startedResearch = true;
                startedResearchName = available[0]->getName();
                log << "\n";
                log << "[Hostile] " << ai->getName() << " starts research: " << available[0]->getName() << ".";
            }
        }

        // Basic colonization: sometimes colonize another colonizable planet in its home system.
        if (chance(gen) < 0.25) {
            if (!ai->getFleets().empty() && ai->getFleets()[0] && ai->getFleets()[0]->getLocation()) {
                auto sys = ai->getFleets()[0]->getLocation();
                auto colonizable = sys->getColonizablePlanets();
                if (!colonizable.empty()) {
                    auto planet = colonizable[0];
                    auto colony = std::make_shared<Colony>(ai->getName() + " Colony " + planet->getName(), planet);
                    planet->colonize(colony);
                    ai->addColony(colony);
                    colonizedPlanets++;
                    log << "\n";
                    log << "[Hostile] " << ai->getName() << " colonizes " << planet->getName() << ".";
                }
            }
        }

        // AI shipbuilding (simple): sometimes add a ship to its first fleet.
        auto& aiFleets = ai->getFleets();
        if (!aiFleets.empty() && aiFleets[0]) {
            if (chance(gen) < 0.45) {
                ShipClass build = aiPickBuildClass(ai->getTurn(), gen);
                const int shipIndex = static_cast<int>(aiFleets[0]->getShips().size() + 1);
                aiFleets[0]->addShip(makeShipForClass(*ai, ai->getName(), build, shipIndex));
                builtShips++;
                log << "\n";
                log << "[Hostile] " << ai->getName() << " builds a " << shipClassToString(build) << ".";
            }
        }

        // AI attacks: occasionally simulate a battle against a random player fleet.
        if (isHostileAtWar(ai->getName()) && chance(gen) < 0.25) {
            auto aiFleet = pickRandomOperationalFleet(ai->getFleets(), gen);
            auto playerFleet = pickRandomOperationalFleet(empire->getFleets(), gen);
            if (aiFleet && playerFleet) {
                attacked = true;
                const int attackerHP0 = fleetTotalHP(aiFleet);
                const int defenderHP0 = fleetTotalHP(playerFleet);
                const int attackerShips0 = fleetShipCount(aiFleet);
                const int defenderShips0 = fleetShipCount(playerFleet);

                Combat combat(aiFleet, playerFleet);
                auto winner = combat.resolve(6);

                log << "\n\n";
                log << "[Hostile Attack] " << ai->getName() << " attacks " << empire->getName() << "!\n";
                log << "Attacker: " << aiFleet->getName() << " vs Defender: " << playerFleet->getName() << "\n";
                log << "Pre-battle HP: " << attackerHP0 << " vs " << defenderHP0 << "\n";
                for (const auto& line : combat.getLog()) {
                    log << line << "\n";
                }
                if (winner) {
                    const int attackerShips1 = fleetShipCount(aiFleet);
                    const int defenderShips1 = fleetShipCount(playerFleet);
                    log << "Winner: " << winner->getName() << "\n";
                    log << "Post-battle ships: " << attackerShips1 << "/" << attackerShips0
                        << " vs " << defenderShips1 << "/" << defenderShips0;

                    // Salvage: winner gets minerals based on defeated side initial HP.
                    int salvage = 0;
                    if (winner.get() == aiFleet.get()) salvage = defenderHP0 / 10;
                    if (winner.get() == playerFleet.get()) salvage = attackerHP0 / 10;
                    if (salvage > 0) {
                        if (winner->getOwner() == empire->getName()) {
                            empire->getResources().add(ResourceType::MINERALS, salvage);
                        } else {
                            for (auto& h : hostileEmpires) {
                                if (h && h->getName() == winner->getOwner()) {
                                    h->getResources().add(ResourceType::MINERALS, salvage);
                                    break;
                                }
                            }
                        }
                        log << "\nSalvage gained: " << salvage << " Minerals";
                    }
                }
            }
        }

        // Compact summary line (in addition to any detailed log above)
        log << "\n";
        log << "[Hostile Summary] " << ai->getName() << ": ";
        if (startedResearch) {
            log << "Researching " << startedResearchName << "; ";
        }
        log << "Built " << builtShips << ", Colonized " << colonizedPlanets;
        if (isHostileAtWar(ai->getName())) {
            log << ", War: Yes";
        } else {
            log << ", War: No";
        }
        if (attacked) {
            log << ", Attacked.";
        } else {
            log << ".";
        }
    }

    return log.str();
}

std::string Game::exploreSystem(const std::string& systemName) {
    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };

    for (auto& system : galaxy->getSystems()) {
        std::string sysNameLower = system->getName();
        std::string searchNameLower = systemName;
        std::transform(sysNameLower.begin(), sysNameLower.end(), sysNameLower.begin(), toLowerChar);
        std::transform(searchNameLower.begin(), searchNameLower.end(), searchNameLower.begin(), toLowerChar);
        
        if (sysNameLower == searchNameLower) {
            const bool wasExplored = system->isExplored();
            system->explore();

            // Check for hostile presence and trigger contact/war.
            for (const auto& h : hostileEmpires) {
                if (!h) continue;
                for (const auto& f : h->getFleets()) {
                    if (f && f->getLocation() == system) {
                        hostileContacted[h->getName()] = true;
                        hostileAtWar[h->getName()] = true;
                    }
                }
            }

            if (!wasExplored) {
                const int reward = 10 + static_cast<int>(system->getPlanets().size()) * 2;
                empire->getResources().add(ResourceType::RESEARCH_POINTS, reward);
                std::string msg = "Explored " + system->getName() + "! Found " +
                                  std::to_string(system->getPlanets().size()) + " planets. Gained " +
                                  std::to_string(reward) + " research points.";

                for (const auto& h : hostileEmpires) {
                    if (!h) continue;
                    if (isHostileContacted(h->getName()) && isHostileAtWar(h->getName())) {
                        // If contact was just made in this system, this will already be set.
                        for (const auto& f : h->getFleets()) {
                            if (f && f->getLocation() == system) {
                                msg += "\nContact! Hostile presence detected: " + h->getName() + " (WAR)";
                            }
                        }
                    }
                }

                return msg;
            }

            return "System already explored: " + system->getName();
        }
    }
    return "System not found";
}

std::string Game::startResearch(const std::string& techId) {
    if (empire->setResearch(techId)) {
        auto tech = empire->getResearch().getTech(techId);
        if (tech) {
            return "Now researching: " + tech->getName();
        }
    }
    return "Cannot research that technology";
}

std::vector<std::shared_ptr<Technology>> Game::getAvailableResearch() {
    return empire->getResearch().getAvailableTechs();
}

std::string Game::buildShip(ShipClass shipClass, const std::string& fleetName) {
    // Find fleet
    std::shared_ptr<Fleet> targetFleet;
    std::string fleetNameLower = fleetName;
    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    std::transform(fleetNameLower.begin(), fleetNameLower.end(), fleetNameLower.begin(), toLowerChar);
    
    for (auto& fleet : empire->getFleets()) {
        std::string fname = fleet->getName();
        std::transform(fname.begin(), fname.end(), fname.begin(), toLowerChar);
        if (fname == fleetNameLower) {
            targetFleet = fleet;
            break;
        }
    }
    
    if (!targetFleet) {
        return "Fleet not found";
    }
    
    // Simple ship creation
    std::string shipName = shipClassToString(shipClass) + "-" + 
                          std::to_string(targetFleet->getShips().size() + 1);
    
    std::shared_ptr<Ship> ship = makeShipForClass(*empire, shipClassToString(shipClass), shipClass,
                                                  static_cast<int>(targetFleet->getShips().size() + 1));
    
    targetFleet->addShip(ship);
    return "Built " + shipName + " and added to " + targetFleet->getName();
}

std::string Game::simulateCombat(const std::string& fleet1Name, const std::string& fleet2Name) {
    std::shared_ptr<Fleet> fleet1, fleet2;

    const auto toLowerChar = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    
    std::string f1Lower = fleet1Name, f2Lower = fleet2Name;
    std::transform(f1Lower.begin(), f1Lower.end(), f1Lower.begin(), toLowerChar);
    std::transform(f2Lower.begin(), f2Lower.end(), f2Lower.begin(), toLowerChar);
    
    for (auto& fleet : empire->getFleets()) {
        std::string fname = fleet->getName();
        std::transform(fname.begin(), fname.end(), fname.begin(), toLowerChar);
        if (fname == f1Lower) fleet1 = fleet;
        if (fname == f2Lower) fleet2 = fleet;
    }
    
    if (!fleet1 || !fleet2) {
        return "One or both fleets not found";
    }
    
    const int hp1 = fleetTotalHP(fleet1);
    const int hp2 = fleetTotalHP(fleet2);
    const int ships1 = fleetShipCount(fleet1);
    const int ships2 = fleetShipCount(fleet2);

    Combat combat(fleet1, fleet2);
    auto winner = combat.resolve();

    showBattleSprites("Battle: " + fleet1->getName() + " vs " + fleet2->getName(), combat.getFrames());
    
    std::string result;
    result += "Pre-battle: " + fleet1->getName() + " (Ships " + std::to_string(ships1) + ", HP " + std::to_string(hp1) + ") vs " +
              fleet2->getName() + " (Ships " + std::to_string(ships2) + ", HP " + std::to_string(hp2) + ")\n\n";
    for (const auto& log : combat.getLog()) {
        result += log + "\n";
    }

    const int ships1After = fleetShipCount(fleet1);
    const int ships2After = fleetShipCount(fleet2);
    result += "\nPost-battle ships: " + std::to_string(ships1After) + "/" + std::to_string(ships1) + " vs " +
              std::to_string(ships2After) + "/" + std::to_string(ships2) + "\n";
    if (winner) {
        result += "Winner: " + winner->getName() + "\n";
    }
    return result;
}
