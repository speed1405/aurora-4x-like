#include "ui.h"
#include <iostream>
#include <sstream>
#include <limits>

UIManager::UIManager() 
    : initialized(false), mouseEnabled(false), selectedItem(0) {
#ifdef USE_NCURSES
    mainWin = nullptr;
#endif
}

UIManager::~UIManager() {
    cleanup();
}

void UIManager::init() {
    if (initialized) return;
    
#ifdef USE_NCURSES
    // Initialize ncurses
    mainWin = initscr();
    cbreak();           // Disable line buffering
    noecho();           // Don't echo typed characters
    keypad(stdscr, TRUE);  // Enable function keys
    
    // Try to enable mouse support
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    
    // Check if mouse is actually available
    if (has_mouse()) {
        mouseEnabled = true;
    } else {
        mouseEnabled = false;
    }
    
    initialized = true;
#else
    // Fallback: no ncurses, use standard I/O
    initialized = true;
    mouseEnabled = false;
#endif
}

void UIManager::cleanup() {
    if (!initialized) return;
    
#ifdef USE_NCURSES
    if (mainWin) {
        endwin();
        mainWin = nullptr;
    }
#endif
    
    initialized = false;
}

void UIManager::clear() {
#ifdef USE_NCURSES
    if (initialized) {
        ::clear();
        refresh();
    }
#else
    // Clear console using ANSI escape codes or system command
    std::cout << "\033[2J\033[1;1H";
#endif
}

bool UIManager::hasMouseSupport() const {
    return mouseEnabled;
}

void UIManager::drawMenuItem(const MenuItem& item, bool highlighted) {
#ifdef USE_NCURSES
    if (!initialized) return;
    
    if (highlighted) {
        attron(A_REVERSE);  // Reverse video for highlighting
    }
    
    mvprintw(item.y, item.x, "%s", item.label.c_str());
    
    if (highlighted) {
        attroff(A_REVERSE);
    }
#else
    // Fallback: simple text output
    if (highlighted) {
        std::cout << "> ";
    } else {
        std::cout << "  ";
    }
    std::cout << item.label << std::endl;
#endif
}

void UIManager::refreshScreen() {
#ifdef USE_NCURSES
    if (initialized) {
        refresh();
    }
#endif
}

int UIManager::displayMenu(const std::string& title, const std::vector<MenuItem>& items) {
    if (!initialized) {
        init();
    }
    
#ifdef USE_NCURSES
    clear();
    
    // Display title
    int titleY = 2;
    int titleX = (COLS - title.length()) / 2;
    mvprintw(titleY, titleX, "%s", title.c_str());
    mvprintw(titleY + 1, 0, "%s", std::string(COLS, '=').c_str());
    
    // Position menu items
    int startY = titleY + 3;
    int menuX = 5;
    
    std::vector<MenuItem> positionedItems = items;
    for (size_t i = 0; i < positionedItems.size(); ++i) {
        positionedItems[i].y = startY + i * 2;
        positionedItems[i].x = menuX;
    }
    
    selectedItem = 0;
    
    // Display hint
    int hintY = startY + positionedItems.size() * 2 + 2;
    if (mouseEnabled) {
        mvprintw(hintY, menuX, "Use mouse to click or arrow keys + Enter to select");
    } else {
        mvprintw(hintY, menuX, "Use arrow keys + Enter to select");
    }
    
    // Main input loop
    while (true) {
        // Draw all menu items
        for (size_t i = 0; i < positionedItems.size(); ++i) {
            drawMenuItem(positionedItems[i], i == static_cast<size_t>(selectedItem));
        }
        refreshScreen();
        
        int ch = getch();
        
        if (ch == KEY_MOUSE && mouseEnabled) {
            MEVENT event;
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_CLICKED) {
                    // Check if click is on a menu item
                    for (size_t i = 0; i < positionedItems.size(); ++i) {
                        if (event.y == positionedItems[i].y &&
                            event.x >= positionedItems[i].x &&
                            event.x < positionedItems[i].x + positionedItems[i].width) {
                            if (positionedItems[i].enabled) {
                                return static_cast<int>(i);
                            }
                        }
                    }
                }
            }
        } else if (ch == KEY_UP) {
            if (selectedItem > 0) {
                selectedItem--;
            }
        } else if (ch == KEY_DOWN) {
            if (selectedItem < static_cast<int>(positionedItems.size()) - 1) {
                selectedItem++;
            }
        } else if (ch == '\n' || ch == 13) {  // Enter key (newline or carriage return)
            if (positionedItems[selectedItem].enabled) {
                return selectedItem;
            }
        } else if (ch == 'q' || ch == 'Q' || ch == 27) {  // ESC key
            return -1;  // Cancel/back
        }
    }
#else
    // Fallback: keyboard-only menu
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
    
    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << (i + 1) << ". " << items[i].label << "\n";
    }
    
    std::cout << "\nEnter choice (1-" << items.size() << "): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (choice >= 1 && choice <= static_cast<int>(items.size())) {
        return choice - 1;
    }
    return -1;
#endif
}

void UIManager::displayText(const std::string& text, bool waitForInput) {
    if (!initialized) {
        init();
    }
    
#ifdef USE_NCURSES
    clear();
    mvprintw(2, 2, "%s", text.c_str());
    
    if (waitForInput) {
        mvprintw(LINES - 2, 2, "Press any key to continue...");
        refreshScreen();
        getch();
    } else {
        refreshScreen();
    }
#else
    std::cout << "\n" << text << "\n";
    if (waitForInput) {
        std::cout << "Press Enter to continue...";
        std::cin.get();
    }
#endif
}

std::string UIManager::getInput(const std::string& prompt) {
    if (!initialized) {
        init();
    }
    
#ifdef USE_NCURSES
    echo();  // Enable echo for input
    char buffer[256];
    mvprintw(LINES - 2, 2, "%s", prompt.c_str());
    refreshScreen();
    getnstr(buffer, sizeof(buffer) - 1);
    noecho();  // Disable echo again
    return std::string(buffer);
#else
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
#endif
}

int UIManager::getIntInput(const std::string& prompt, int defaultValue) {
    std::string input = getInput(prompt);
    if (input.empty()) {
        return defaultValue;
    }
    
    try {
        return std::stoi(input);
    } catch (...) {
        return defaultValue;
    }
}
