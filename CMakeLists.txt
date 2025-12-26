cmake_minimum_required(VERSION 3.10)
project(Aurora4XLike VERSION 0.1.0 LANGUAGES CXX)

option(AURORA_WINDOWS_GUI "Build Windows mouse-driven GUI (Win32)" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/ui.cpp
    src/resources.cpp
    src/research.cpp
    src/empire.cpp
    src/combat.cpp
    src/galaxy.cpp
    src/game.cpp
)

if (WIN32)
    list(APPEND SOURCES src/battle_viewer_win32.cpp)

    if (AURORA_WINDOWS_GUI)
        list(REMOVE_ITEM SOURCES src/main.cpp)
        list(APPEND SOURCES src/main_win32_gui.cpp src/gui_win32.cpp)
    endif()
endif()

# Executable
if (WIN32 AND AURORA_WINDOWS_GUI)
    add_executable(aurora4x WIN32 ${SOURCES})
else()
    add_executable(aurora4x ${SOURCES})
endif()

# Find and link ncurses library for mouse support (Unix-like systems only)
if(UNIX)
    find_package(Curses REQUIRED)
    if(CURSES_FOUND)
        include_directories(${CURSES_INCLUDE_DIR})
        target_link_libraries(aurora4x ${CURSES_LIBRARIES})
        add_definitions(-DUSE_NCURSES)
    endif()
endif()

# Compiler warnings
if(MSVC)
    # CMake's MSVC defaults include /W3; replace it with /W4 to avoid D9025
    foreach(flag_var
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_MINSIZEREL)
        if(DEFINED ${flag_var})
            string(REGEX REPLACE "/W[0-4]" "/W4" _tmp "${${flag_var}}")
            set(${flag_var} "${_tmp}" CACHE STRING "" FORCE)
        endif()
    endforeach()
else()
    target_compile_options(aurora4x PRIVATE -Wall -Wextra -pedantic)
endif()
