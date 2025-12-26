#include "ui.h"
#include <iostream>
#include <sstream>
#include <limits>

#if !defined(USE_NCURSES) && defined(_WIN32)
#include <string>

static bool winGetConsoleSize(HANDLE hOut, int& cols, int& rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        return false;
    }
    cols = static_cast<int>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    rows = static_cast<int>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    return true;
}

static void winSetCursor(HANDLE hOut, int x, int y) {
    COORD pos;
    pos.X = static_cast<SHORT>(x);
    pos.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(hOut, pos);
}

static void winWriteAt(HANDLE hOut, int x, int y, const std::string& text) {
    winSetCursor(hOut, x, y);
    DWORD written = 0;
    WriteConsoleA(hOut, text.c_str(), static_cast<DWORD>(text.size()), &written, nullptr);
}

static void winClearScreen(HANDLE hOut) {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        return;
    }

    const DWORD cellCount = static_cast<DWORD>(csbi.dwSize.X) * static_cast<DWORD>(csbi.dwSize.Y);
    COORD home{0, 0};

    DWORD written = 0;
    FillConsoleOutputCharacterA(hOut, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, cellCount, home, &written);
    SetConsoleCursorPosition(hOut, home);
}
#endif

UIManager::UIManager() 
    : initialized(false), mouseEnabled(false), selectedItem(0) {
#ifdef USE_NCURSES
    mainWin = nullptr;
#endif

#if !defined(USE_NCURSES) && defined(_WIN32)
    hIn = nullptr;
    hOut = nullptr;
    originalInMode = 0;
    originalOutAttributes = 0;
    winConsoleAvailable = false;
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
    // Fallback: no ncurses
    initialized = true;

    // On Windows, use Win32 console mouse events to enable clickable menus.
#if defined(_WIN32)
    hIn = GetStdHandle(STD_INPUT_HANDLE);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    winConsoleAvailable = (hIn != nullptr && hIn != INVALID_HANDLE_VALUE &&
                           hOut != nullptr && hOut != INVALID_HANDLE_VALUE);

    mouseEnabled = false;
    if (winConsoleAvailable) {
        DWORD mode = 0;
        if (GetConsoleMode(hIn, &mode)) {
            originalInMode = mode;

            // Keep existing input behavior (line input, echo, etc.) but enable mouse.
            // Also disable Quick Edit to prevent click-to-select from freezing input.
            DWORD newMode = mode;
            newMode |= ENABLE_EXTENDED_FLAGS;
            newMode |= ENABLE_MOUSE_INPUT;
            newMode |= ENABLE_WINDOW_INPUT;
            newMode &= ~ENABLE_QUICK_EDIT_MODE;

            if (SetConsoleMode(hIn, newMode)) {
                mouseEnabled = true;
            }
        }

        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            originalOutAttributes = csbi.wAttributes;
        }
    }
#else
    mouseEnabled = false;
#endif
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

#if !defined(USE_NCURSES) && defined(_WIN32)
    if (winConsoleAvailable) {
        if (originalInMode != 0) {
            SetConsoleMode(hIn, originalInMode);
        }

        if (originalOutAttributes != 0) {
            SetConsoleTextAttribute(hOut, originalOutAttributes);
        }
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
    // Clear console.
#if defined(_WIN32)
    if (winConsoleAvailable) {
        winClearScreen(hOut);
    } else {
        std::cout << "\033[2J\033[1;1H";
    }
#else
    std::cout << "\033[2J\033[1;1H";
#endif
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
    // Non-ncurses UI
#if defined(_WIN32)
    if (winConsoleAvailable) {
        const int markerX = (item.x >= 2) ? (item.x - 2) : 0;
        winWriteAt(hOut, markerX, item.y, highlighted ? "> " : "  ");
        winWriteAt(hOut, item.x, item.y, item.label);
        return;
    }
#endif

    // Generic fallback: simple text output
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
        // Update width to account for actual label length
        positionedItems[i].width = positionedItems[i].label.length();
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
    // Non-ncurses menu
#if defined(_WIN32)
    if (winConsoleAvailable) {
        clear();

        int cols = 80;
        int rows = 25;
        winGetConsoleSize(hOut, cols, rows);

        const int titleY = 1;
        int titleX = 0;
        if (cols > static_cast<int>(title.size())) {
            titleX = (cols - static_cast<int>(title.size())) / 2;
        }
        winWriteAt(hOut, titleX, titleY, title);
        winWriteAt(hOut, 0, titleY + 1, std::string(cols, '='));

        const int startY = titleY + 3;
        const int labelX = 6;

        std::vector<MenuItem> positionedItems = items;
        for (size_t i = 0; i < positionedItems.size(); ++i) {
            positionedItems[i].y = startY + static_cast<int>(i) * 2;
            positionedItems[i].x = labelX;
            positionedItems[i].width = static_cast<int>(positionedItems[i].label.length());
        }

        selectedItem = 0;

        const int hintY = startY + static_cast<int>(positionedItems.size()) * 2 + 1;
        if (mouseEnabled) {
            winWriteAt(hOut, 2, hintY, "Click a menu item (or use arrows + Enter). Q/Esc to go back.");
        } else {
            winWriteAt(hOut, 2, hintY, "Use arrows + Enter. Q/Esc to go back.");
        }

        while (true) {
            for (size_t i = 0; i < positionedItems.size(); ++i) {
                drawMenuItem(positionedItems[i], i == static_cast<size_t>(selectedItem));
            }
            refreshScreen();

            INPUT_RECORD record{};
            DWORD read = 0;
            if (!ReadConsoleInput(hIn, &record, 1, &read) || read == 0) {
                return -1;
            }

            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                const WORD vk = record.Event.KeyEvent.wVirtualKeyCode;
                const CHAR ch = record.Event.KeyEvent.uChar.AsciiChar;

                if (vk == VK_UP) {
                    if (selectedItem > 0) {
                        selectedItem--;
                    }
                } else if (vk == VK_DOWN) {
                    if (selectedItem < static_cast<int>(positionedItems.size()) - 1) {
                        selectedItem++;
                    }
                } else if (vk == VK_RETURN) {
                    if (!positionedItems.empty() && positionedItems[selectedItem].enabled) {
                        return selectedItem;
                    }
                } else if (vk == VK_ESCAPE || ch == 'q' || ch == 'Q') {
                    return -1;
                }
            } else if (record.EventType == MOUSE_EVENT && mouseEnabled) {
                const MOUSE_EVENT_RECORD& me = record.Event.MouseEvent;
                const int mx = static_cast<int>(me.dwMousePosition.X);
                const int my = static_cast<int>(me.dwMousePosition.Y);

                // Hover: update selection when cursor is over an item.
                for (size_t i = 0; i < positionedItems.size(); ++i) {
                    const auto& it = positionedItems[i];
                    if (my == it.y && mx >= it.x && mx < it.x + it.width) {
                        selectedItem = static_cast<int>(i);
                        break;
                    }
                }

                // Click: select item.
                if (me.dwEventFlags == 0 && (me.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
                    if (!positionedItems.empty() && positionedItems[selectedItem].enabled) {
                        const auto& it = positionedItems[static_cast<size_t>(selectedItem)];
                        if (my == it.y && mx >= it.x && mx < it.x + it.width) {
                            return selectedItem;
                        }
                    }
                }
            }
        }
    }
#endif

    // Generic fallback: keyboard-only menu
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
