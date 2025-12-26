#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "game.h"

namespace {

enum class View {
    Status,
    Research,
    Explore,
    Fleet,
    Combat,
    Hostiles,
    Help,
};

enum ControlId : int {
    NAV_STATUS = 100,
    NAV_RESEARCH,
    NAV_EXPLORE,
    NAV_FLEET,
    NAV_COMBAT,
    NAV_HOSTILES,
    NAV_ADVANCE,
    NAV_HELP,
    NAV_QUIT,

    LIST_MAIN = 200,
    BTN_ACTION = 201,

    COMBO_FLEET_A = 210,
    COMBO_FLEET_B = 211,
    COMBO_SHIP_CLASS = 212,

    EDIT_LOG = 300,
    STATIC_TITLE = 301,
};

struct AppState {
    std::unique_ptr<Game> game;
    View view{View::Status};

    HWND hwnd{};

    // Navigation
    HWND btnStatus{};
    HWND btnResearch{};
    HWND btnExplore{};
    HWND btnFleet{};
    HWND btnCombat{};
    HWND btnHostiles{};
    HWND btnAdvance{};
    HWND btnHelp{};
    HWND btnQuit{};

    // Content
    HWND staticTitle{};
    HWND listMain{};
    HWND btnAction{};

    HWND comboFleetA{};
    HWND comboFleetB{};
    HWND comboShipClass{};

    // Log
    HWND editLog{};

    // Cached data backing list/combos
    std::vector<std::shared_ptr<Technology>> availableTechs;
    std::vector<std::shared_ptr<StarSystem>> unexploredSystems;
    std::vector<std::shared_ptr<Fleet>> fleets;
    std::vector<std::shared_ptr<Empire>> hostiles;

    // UI selection memory (avoid cross-view selection bleed)
    std::string selectedTechId;
    std::string selectedSystemName;
    std::string selectedHostileName;
};

static bool systemHasHostiles(Game& game, const std::shared_ptr<StarSystem>& sys) {
    if (!sys) return false;
    for (const auto& h : game.getHostileEmpires()) {
        if (!h) continue;
        for (const auto& f : h->getFleets()) {
            if (f && f->getLocation() == sys) return true;
        }
    }
    return false;
}

static void setEditText(HWND edit, const std::string& text) {
    SetWindowTextA(edit, text.c_str());
}

static std::vector<std::string> splitLinesForListbox(const std::string& text) {
    std::vector<std::string> lines;
    std::string cur;
    cur.reserve(128);

    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            lines.push_back(cur);
            cur.clear();
            continue;
        }
        cur.push_back(c);
    }
    if (!cur.empty() || lines.empty()) {
        lines.push_back(cur);
    }
    return lines;
}

static std::string normalizeToCrlf(std::string text) {
    // Convert lone \n to \r\n; preserve existing \r\n.
    std::string out;
    out.reserve(text.size() + 16);
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '\n') {
            if (i == 0 || text[i - 1] != '\r') out.push_back('\r');
            out.push_back('\n');
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static std::string getComboSelectedText(HWND combo) {
    if (!combo) return {};
    const int sel = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return {};
    const int len = (int)SendMessageA(combo, CB_GETLBTEXTLEN, (WPARAM)sel, 0);
    if (len <= 0) return {};
    std::string buf;
    buf.resize((size_t)len + 1);
    SendMessageA(combo, CB_GETLBTEXT, (WPARAM)sel, (LPARAM)buf.data());
    if (!buf.empty() && buf.back() == '\0') buf.pop_back();
    return buf;
}

static void appendLog(AppState& s, const std::string& line) {
    if (!s.editLog) return;

    int len = GetWindowTextLengthA(s.editLog);
    SendMessageA(s.editLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    std::string msg = normalizeToCrlf(line);
    if (!msg.empty() && msg.size() >= 2) {
        const char last = msg.back();
        const char prev = msg[msg.size() - 2];
        if (!(prev == '\r' && last == '\n')) {
            msg.append("\r\n");
        }
    } else if (!msg.empty() && msg.back() != '\n') {
        msg.append("\r\n");
    }
    SendMessageA(s.editLog, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());
}

static void clearLog(AppState& s) {
    setEditText(s.editLog, "");
}

static std::string makeEmpireStatus(Game& game) {
    auto empire = game.getEmpire();

    std::ostringstream oss;
    oss << "EMPIRE: " << empire->getName() << "\r\n\r\n";
    oss << "Turn: " << empire->getTurn() << "\r\n";
    oss << "Colonies: " << empire->getColonies().size() << "\r\n";
    oss << "Fleets: " << empire->getFleets().size() << "\r\n";
    oss << "Researched Technologies: " << empire->getResearch().getResearchedCount() << "\r\n";

    if (empire->getCurrentResearch().empty()) {
        oss << "Current Research: None\r\n";
    } else {
        auto tech = empire->getResearch().getTech(empire->getCurrentResearch());
        if (tech) {
            oss << "Current Research: " << tech->getName() << " (" << tech->getProgress() << "/" << tech->getCost() << ")\r\n";
        } else {
            oss << "Current Research: " << empire->getCurrentResearch() << "\r\n";
        }
    }

    oss << "\r\nResources:\r\n";
    oss << "  Minerals: " << empire->getResources().get(ResourceType::MINERALS) << "\r\n";
    oss << "  Energy: " << empire->getResources().get(ResourceType::ENERGY) << "\r\n";
    oss << "  Research Points: " << empire->getResources().get(ResourceType::RESEARCH_POINTS) << "\r\n";

    return oss.str();
}

static void refreshFleetCache(AppState& s) {
    const std::string prevAName = getComboSelectedText(s.comboFleetA);
    const std::string prevBName = getComboSelectedText(s.comboFleetB);

    s.fleets = s.game->getEmpire()->getFleets();

    auto refillCombo = [&](HWND combo, const std::string& prevName) {
        if (!combo) return;
        SendMessageA(combo, CB_RESETCONTENT, 0, 0);
        for (size_t i = 0; i < s.fleets.size(); ++i) {
            SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)s.fleets[i]->getName().c_str());
        }
        if (s.fleets.empty()) return;

        int sel = 0;
        if (!prevName.empty()) {
            for (size_t i = 0; i < s.fleets.size(); ++i) {
                if (s.fleets[i] && s.fleets[i]->getName() == prevName) {
                    sel = (int)i;
                    break;
                }
            }
        }
        SendMessageA(combo, CB_SETCURSEL, (WPARAM)sel, 0);
    };

    refillCombo(s.comboFleetA, prevAName);
    refillCombo(s.comboFleetB, prevBName);
}

static ShipClass shipClassFromComboIndex(int idx) {
    switch (idx) {
        case 0: return ShipClass::FIGHTER;
        case 1: return ShipClass::SCOUT;
        case 2: return ShipClass::CORVETTE;
        case 3: return ShipClass::FRIGATE;
        case 4: return ShipClass::DESTROYER;
        case 5: return ShipClass::CRUISER;
        case 6: return ShipClass::BATTLESHIP;
        case 7: return ShipClass::CARRIER;
        default: return ShipClass::SCOUT;
    }
}

static void refreshContent(AppState& s, bool resetLog) {
    if (!s.game) return;

    // Common: ensure caches are fresh
    refreshFleetCache(s);

    // Hide/show optional controls by view
    auto show = [](HWND w, bool on) {
        if (!w) return;
        ShowWindow(w, on ? SW_SHOW : SW_HIDE);
        EnableWindow(w, on ? TRUE : FALSE);
    };

    show(s.listMain, s.view == View::Status || s.view == View::Help || s.view == View::Research || s.view == View::Explore || s.view == View::Hostiles);
    show(s.btnAction, s.view == View::Research || s.view == View::Explore || s.view == View::Fleet || s.view == View::Combat || s.view == View::Hostiles);

    show(s.comboFleetA, s.view == View::Fleet || s.view == View::Combat);
    show(s.comboFleetB, s.view == View::Combat);
    show(s.comboShipClass, s.view == View::Fleet);

    SendMessageA(s.listMain, LB_RESETCONTENT, 0, 0);

    switch (s.view) {
        case View::Status: {
            SetWindowTextA(s.staticTitle, "Empire Status");
            const auto lines = splitLinesForListbox(makeEmpireStatus(*s.game));
            for (const auto& ln : lines) {
                SendMessageA(s.listMain, LB_ADDSTRING, 0, (LPARAM)ln.c_str());
            }
            (void)resetLog;
            break;
        }
        case View::Help: {
            SetWindowTextA(s.staticTitle, "Help");
            const char* helpLines[] = {
                "AURORA 4X-LIKE",
                "",
                "- Use the buttons on the left to navigate.",
                "- Research: select a tech, click Start Research.",
                "- Explore: select a system, click Explore.",
                "- Fleet: choose a fleet + ship class, click Build Ship.",
                "- Combat: choose two fleets, click Simulate Combat.",
                "- Hostiles: inspect known hostile empires.",
                "- Advance Turn: progresses the empire one turn.",
            };
            for (const char* ln : helpLines) {
                SendMessageA(s.listMain, LB_ADDSTRING, 0, (LPARAM)ln);
            }
            (void)resetLog;
            break;
        }
        case View::Research: {
            SetWindowTextA(s.staticTitle, "Research");
            SetWindowTextA(s.btnAction, "Start Research");
            if (resetLog) {
                clearLog(s);
                appendLog(s, makeEmpireStatus(*s.game));
            }

            s.availableTechs = s.game->getAvailableResearch();
            int restoreListIndex = -1;
            for (size_t i = 0; i < s.availableTechs.size(); ++i) {
                auto tech = s.availableTechs[i];
                if (!tech) continue;
                std::ostringstream label;
                label << tech->getName() << " (" << techCategoryToString(tech->getCategory())
                      << ", Cost: " << tech->getCost() << " RP)";
                int idx = (int)SendMessageA(s.listMain, LB_ADDSTRING, 0, (LPARAM)label.str().c_str());
                SendMessageA(s.listMain, LB_SETITEMDATA, idx, (LPARAM)i);

                if (!s.selectedTechId.empty() && tech->getId() == s.selectedTechId) {
                    restoreListIndex = idx;
                }
            }
            if (!s.availableTechs.empty()) {
                if (restoreListIndex < 0) restoreListIndex = 0;
                SendMessageA(s.listMain, LB_SETCURSEL, (WPARAM)restoreListIndex, 0);
            }
            break;
        }
        case View::Explore: {
            SetWindowTextA(s.staticTitle, "Explore");
            SetWindowTextA(s.btnAction, "Explore System");
            if (resetLog) {
                clearLog(s);
                appendLog(s, makeEmpireStatus(*s.game));
            }

            auto galaxy = s.game->getGalaxy();
            s.unexploredSystems = galaxy->getUnexploredSystems();
            int restoreListIndex = -1;
            for (size_t i = 0; i < s.unexploredSystems.size(); ++i) {
                auto sys = s.unexploredSystems[i];
                if (!sys) continue;
                std::ostringstream label;
                label << sys->getName() << " (" << sys->getX() << "," << sys->getY() << "," << sys->getZ() << ")";
                if (systemHasHostiles(*s.game, sys)) {
                    label << " [Hostiles]";
                }
                int idx = (int)SendMessageA(s.listMain, LB_ADDSTRING, 0, (LPARAM)label.str().c_str());
                SendMessageA(s.listMain, LB_SETITEMDATA, idx, (LPARAM)i);

                if (!s.selectedSystemName.empty() && sys->getName() == s.selectedSystemName) {
                    restoreListIndex = idx;
                }
            }
            if (!s.unexploredSystems.empty()) {
                if (restoreListIndex < 0) restoreListIndex = 0;
                SendMessageA(s.listMain, LB_SETCURSEL, (WPARAM)restoreListIndex, 0);
            }
            break;
        }
        case View::Fleet: {
            SetWindowTextA(s.staticTitle, "Fleet");
            SetWindowTextA(s.btnAction, "Build Ship");
            if (resetLog) {
                clearLog(s);
                appendLog(s, makeEmpireStatus(*s.game));
            }

            // Populate ship class combo
            if (SendMessageA(s.comboShipClass, CB_GETCOUNT, 0, 0) == 0) {
                const char* items[] = {"Fighter", "Scout", "Corvette", "Frigate", "Destroyer", "Cruiser", "Battleship", "Carrier"};
                for (const char* it : items) {
                    SendMessageA(s.comboShipClass, CB_ADDSTRING, 0, (LPARAM)it);
                }
                SendMessageA(s.comboShipClass, CB_SETCURSEL, 0, 0);
            }

            // Fleet combo already populated in refreshFleetCache
            break;
        }
        case View::Combat: {
            SetWindowTextA(s.staticTitle, "Combat");
            SetWindowTextA(s.btnAction, "Simulate Combat");
            if (resetLog) {
                clearLog(s);
                appendLog(s, makeEmpireStatus(*s.game));
            }

            // Default to two different fleets when possible
            if (s.fleets.size() >= 2) {
                const int aSel = (int)SendMessageA(s.comboFleetA, CB_GETCURSEL, 0, 0);
                const int bSel = (int)SendMessageA(s.comboFleetB, CB_GETCURSEL, 0, 0);
                if (aSel == bSel) {
                    SendMessageA(s.comboFleetB, CB_SETCURSEL, (WPARAM)((aSel == 0) ? 1 : 0), 0);
                }
            }
            break;
        }
        case View::Hostiles: {
            SetWindowTextA(s.staticTitle, "Hostile Empires");
            SetWindowTextA(s.btnAction, "Inspect");
            if (resetLog) {
                clearLog(s);
                appendLog(s, makeEmpireStatus(*s.game));
                appendLog(s, "");
            }

            s.hostiles = s.game->getHostileEmpires();
            int restoreListIndex = -1;
            for (size_t i = 0; i < s.hostiles.size(); ++i) {
                auto h = s.hostiles[i];
                if (!h) continue;
                std::ostringstream label;
                label << h->getName();
                label << (s.game->isHostileAtWar(h->getName()) ? " [WAR]" : " [No war]");
                int idx = (int)SendMessageA(s.listMain, LB_ADDSTRING, 0, (LPARAM)label.str().c_str());
                SendMessageA(s.listMain, LB_SETITEMDATA, idx, (LPARAM)i);

                if (!s.selectedHostileName.empty() && h->getName() == s.selectedHostileName) {
                    restoreListIndex = idx;
                }
            }
            if (!s.hostiles.empty()) {
                if (restoreListIndex < 0) restoreListIndex = 0;
                SendMessageA(s.listMain, LB_SETCURSEL, (WPARAM)restoreListIndex, 0);
            }
            break;
        }
    }
}

static void snapshotListSelection(AppState& s) {
    if (!s.listMain) return;
    const int sel = (int)SendMessageA(s.listMain, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) return;
    const LRESULT data = SendMessageA(s.listMain, LB_GETITEMDATA, (WPARAM)sel, 0);
    if (data == LB_ERR) return;
    const size_t index = (size_t)data;

    switch (s.view) {
        case View::Research:
            if (index < s.availableTechs.size() && s.availableTechs[index]) {
                s.selectedTechId = s.availableTechs[index]->getId();
            }
            break;
        case View::Explore:
            if (index < s.unexploredSystems.size() && s.unexploredSystems[index]) {
                s.selectedSystemName = s.unexploredSystems[index]->getName();
            }
            break;
        case View::Hostiles:
            if (index < s.hostiles.size() && s.hostiles[index]) {
                s.selectedHostileName = s.hostiles[index]->getName();
            }
            break;
        default:
            break;
    }
}

static void doAction(AppState& s) {
    if (!s.game) return;

    switch (s.view) {
        case View::Research: {
            int sel = (int)SendMessageA(s.listMain, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR) {
                appendLog(s, "Select a technology first.");
                return;
            }
            size_t index = (size_t)SendMessageA(s.listMain, LB_GETITEMDATA, sel, 0);
            if (index >= s.availableTechs.size() || !s.availableTechs[index]) {
                appendLog(s, "Invalid selection.");
                return;
            }
            std::string result = s.game->startResearch(s.availableTechs[index]->getId());
            appendLog(s, "");
            appendLog(s, result);
            refreshContent(s, false);
            return;
        }
        case View::Explore: {
            int sel = (int)SendMessageA(s.listMain, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR) {
                appendLog(s, "Select a system first.");
                return;
            }
            size_t index = (size_t)SendMessageA(s.listMain, LB_GETITEMDATA, sel, 0);
            if (index >= s.unexploredSystems.size() || !s.unexploredSystems[index]) {
                appendLog(s, "Invalid selection.");
                return;
            }
            std::string name = s.unexploredSystems[index]->getName();
            std::string result = s.game->exploreSystem(name);
            appendLog(s, "");
            appendLog(s, result);
            refreshContent(s, false);
            return;
        }
        case View::Fleet: {
            int fleetSel = (int)SendMessageA(s.comboFleetA, CB_GETCURSEL, 0, 0);
            int classSel = (int)SendMessageA(s.comboShipClass, CB_GETCURSEL, 0, 0);
            if (fleetSel == CB_ERR || fleetSel < 0 || (size_t)fleetSel >= s.fleets.size()) {
                appendLog(s, "Select a fleet.");
                return;
            }
            ShipClass sc = shipClassFromComboIndex(classSel);
            std::string result = s.game->buildShip(sc, s.fleets[(size_t)fleetSel]->getName());
            appendLog(s, "");
            appendLog(s, result);
            refreshContent(s, false);
            return;
        }
        case View::Combat: {
            int aSel = (int)SendMessageA(s.comboFleetA, CB_GETCURSEL, 0, 0);
            int bSel = (int)SendMessageA(s.comboFleetB, CB_GETCURSEL, 0, 0);
            if (aSel == CB_ERR || bSel == CB_ERR) {
                appendLog(s, "Select two fleets.");
                return;
            }
            if (aSel == bSel) {
                appendLog(s, "Choose two different fleets.");
                return;
            }
            if (aSel < 0 || bSel < 0 || (size_t)aSel >= s.fleets.size() || (size_t)bSel >= s.fleets.size()) {
                appendLog(s, "Invalid fleet selection.");
                return;
            }

            std::string result = s.game->simulateCombat(s.fleets[(size_t)aSel]->getName(), s.fleets[(size_t)bSel]->getName());
            appendLog(s, "");
            appendLog(s, result);
            refreshContent(s, false);
            return;
        }
        case View::Hostiles: {
            int sel = (int)SendMessageA(s.listMain, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR) {
                appendLog(s, "Select a hostile empire first.");
                return;
            }
            size_t index = (size_t)SendMessageA(s.listMain, LB_GETITEMDATA, sel, 0);
            if (index >= s.hostiles.size() || !s.hostiles[index]) {
                appendLog(s, "Invalid selection.");
                return;
            }
            auto h = s.hostiles[index];
            appendLog(s, "");
            appendLog(s, "--- Hostile Empire ---");
            appendLog(s, "Name: " + h->getName());
            appendLog(s, std::string("At war: ") + (s.game->isHostileAtWar(h->getName()) ? "Yes" : "No"));
            appendLog(s, "Turn: " + std::to_string(h->getTurn()));
            appendLog(s, "Colonies: " + std::to_string(h->getColonies().size()));
            appendLog(s, "Fleets: " + std::to_string(h->getFleets().size()));
            for (const auto& f : h->getFleets()) {
                if (!f) continue;
                appendLog(s, "  Fleet: " + f->getName() + " (Strength: " + std::to_string(f->getCombatStrength()) + ")");
            }
            return;
        }
        default:
            return;
    }
}

static void doAdvanceTurn(AppState& s) {
    std::string result = s.game->advanceTurn();
    appendLog(s, "");
    appendLog(s, result);
    refreshContent(s, false);
}

static void layout(AppState& s) {
    RECT r{};
    GetClientRect(s.hwnd, &r);

    const int w = r.right - r.left;
    const int h = r.bottom - r.top;

    const int navW = 180;
    const int padding = 10;
    const int btnH = 30;
    const int btnGap = 6;

    int y = padding;

    auto placeNav = [&](HWND btn) {
        MoveWindow(btn, padding, y, navW - 2 * padding, btnH, TRUE);
        y += btnH + btnGap;
    };

    placeNav(s.btnStatus);
    placeNav(s.btnResearch);
    placeNav(s.btnExplore);
    placeNav(s.btnFleet);
    placeNav(s.btnCombat);
    placeNav(s.btnHostiles);
    placeNav(s.btnAdvance);
    placeNav(s.btnHelp);
    placeNav(s.btnQuit);

    const int contentX = navW + padding;
    const int contentW = w - contentX - padding;

    const int titleH = 24;
    MoveWindow(s.staticTitle, contentX, padding, contentW, titleH, TRUE);

    const int logH = std::max(180, h / 3);
    const int contentTop = padding + titleH + padding;
    const int contentBottom = h - padding - logH - padding;
    const int contentH = std::max(0, contentBottom - contentTop);

    const int controlsH = 30;
    const int listH = std::max(0, contentH - controlsH - padding);

    // List + action
    MoveWindow(s.listMain, contentX, contentTop, contentW, listH, TRUE);

    const int rowY = contentTop + listH + padding;

    const int comboW = (contentW - padding * 3) / 2;
    MoveWindow(s.comboFleetA, contentX, rowY, comboW, controlsH, TRUE);
    MoveWindow(s.comboFleetB, contentX + comboW + padding, rowY, comboW, controlsH, TRUE);

    MoveWindow(s.comboShipClass, contentX, rowY, comboW, controlsH, TRUE);

    MoveWindow(s.btnAction, contentX + comboW + padding, rowY, comboW, controlsH, TRUE);

    // Log
    MoveWindow(s.editLog, contentX, h - padding - logH, contentW, logH, TRUE);
}

static void setView(AppState& s, View v) {
    snapshotListSelection(s);
    s.view = v;
    refreshContent(s, false);
    layout(s);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* s = reinterpret_cast<AppState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }
        case WM_SIZE: {
            if (s) layout(*s);
            return 0;
        }
        case WM_COMMAND: {
            if (!s) break;
            const int id = LOWORD(wParam);
            const int code = HIWORD(wParam);

            if (code == BN_CLICKED) {
                switch (id) {
                    case NAV_STATUS: setView(*s, View::Status); return 0;
                    case NAV_RESEARCH: setView(*s, View::Research); return 0;
                    case NAV_EXPLORE: setView(*s, View::Explore); return 0;
                    case NAV_FLEET: setView(*s, View::Fleet); return 0;
                    case NAV_COMBAT: setView(*s, View::Combat); return 0;
                    case NAV_HOSTILES: setView(*s, View::Hostiles); return 0;
                    case NAV_HELP: setView(*s, View::Help); return 0;
                    case NAV_ADVANCE: doAdvanceTurn(*s); return 0;
                    case NAV_QUIT: DestroyWindow(hwnd); return 0;
                    case BTN_ACTION: doAction(*s); return 0;
                }
            }
            // Double click list to activate
            if (id == LIST_MAIN && code == LBN_DBLCLK) {
                doAction(*s);
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND makeButton(HWND parent, const char* text, int id) {
    return CreateWindowExA(0, "BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           0, 0, 0, 0, parent, (HMENU)(intptr_t)id, GetModuleHandle(nullptr), nullptr);
}

static HWND makeCombo(HWND parent, int id) {
    return CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
                           0, 0, 0, 0, parent, (HMENU)(intptr_t)id, GetModuleHandle(nullptr), nullptr);
}

static HWND makeList(HWND parent, int id) {
    return CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
                           0, 0, 0, 0, parent, (HMENU)(intptr_t)id, GetModuleHandle(nullptr), nullptr);
}

static HWND makeLog(HWND parent, int id) {
    HWND edit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
                                0, 0, 0, 0, parent, (HMENU)(intptr_t)id, GetModuleHandle(nullptr), nullptr);
    SendMessageA(edit, EM_SETLIMITTEXT, 0, 0);
    return edit;
}

static HWND makeStatic(HWND parent, const char* text, int id) {
    return CreateWindowExA(0, "STATIC", text, WS_CHILD | WS_VISIBLE,
                           0, 0, 0, 0, parent, (HMENU)(intptr_t)id, GetModuleHandle(nullptr), nullptr);
}

} // namespace

int runAuroraGuiWin32() {
    AppState state;
    state.game = std::make_unique<Game>("Earth Empire");

    WNDCLASSA wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "Aurora4xGui";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        return 1;
    }

    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "Aurora 4X-Like",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                1000, 700, nullptr, nullptr, wc.hInstance, &state);
    if (!hwnd) {
        return 1;
    }

    state.hwnd = hwnd;

    // Nav
    state.btnStatus = makeButton(hwnd, "Status", NAV_STATUS);
    state.btnResearch = makeButton(hwnd, "Research", NAV_RESEARCH);
    state.btnExplore = makeButton(hwnd, "Explore", NAV_EXPLORE);
    state.btnFleet = makeButton(hwnd, "Fleet", NAV_FLEET);
    state.btnCombat = makeButton(hwnd, "Combat", NAV_COMBAT);
    state.btnHostiles = makeButton(hwnd, "Hostiles", NAV_HOSTILES);
    state.btnAdvance = makeButton(hwnd, "Advance Turn", NAV_ADVANCE);
    state.btnHelp = makeButton(hwnd, "Help", NAV_HELP);
    state.btnQuit = makeButton(hwnd, "Quit", NAV_QUIT);

    // Content
    state.staticTitle = makeStatic(hwnd, "", STATIC_TITLE);
    state.listMain = makeList(hwnd, LIST_MAIN);
    state.btnAction = makeButton(hwnd, "", BTN_ACTION);
    state.comboFleetA = makeCombo(hwnd, COMBO_FLEET_A);
    state.comboFleetB = makeCombo(hwnd, COMBO_FLEET_B);
    state.comboShipClass = makeCombo(hwnd, COMBO_SHIP_CLASS);

    // Log
    state.editLog = makeLog(hwnd, EDIT_LOG);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&state));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    setView(state, View::Status);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

#else

int runAuroraGuiWin32() {
    return 1;
}

#endif
