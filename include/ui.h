#ifndef UI_H
#define UI_H

#ifdef USE_NCURSES
#include <ncurses.h>
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <functional>

// Menu item structure for mouse-driven UI
struct MenuItem {
    std::string label;
    std::function<void()> action;
    int x, y;  // Position on screen
    int width;
    bool enabled;
    
    MenuItem(const std::string& lbl, std::function<void()> act, bool en = true)
        : label(lbl), action(act), x(0), y(0), width(static_cast<int>(lbl.length())), enabled(en) {}
};

// UI Manager class for handling mouse-driven interface
class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialize the UI system
    void init();
    
    // Clean up the UI system
    void cleanup();
    
    // Display a menu and handle mouse/keyboard input
    int displayMenu(const std::string& title, const std::vector<MenuItem>& items);
    
    // Display text and wait for user input
    void displayText(const std::string& text, bool waitForInput = true);
    
    // Get string input from user
    std::string getInput(const std::string& prompt);
    
    // Get integer input
    int getIntInput(const std::string& prompt, int defaultValue = 0);
    
    // Clear the screen
    void clear();
    
    // Check if mouse support is available
    bool hasMouseSupport() const;
    
private:
    bool initialized;
    bool mouseEnabled;
    int selectedItem;
    
#ifdef USE_NCURSES
    WINDOW* mainWin;
#endif

#if !defined(USE_NCURSES) && defined(_WIN32)
    HANDLE hIn;
    HANDLE hOut;
    DWORD originalInMode;
    WORD originalOutAttributes;
    bool winConsoleAvailable;
#endif
    
    void drawMenuItem(const MenuItem& item, bool highlighted);
    void refreshScreen();
};

#endif // UI_H
