#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

int runAuroraGuiWin32();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return runAuroraGuiWin32();
}
#endif
