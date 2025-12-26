cmake_minimum_required(VERSION 3.10)
project(Aurora4XLike VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/resources.cpp
    src/research.cpp
    src/empire.cpp
    src/combat.cpp
    src/galaxy.cpp
    src/game.cpp
)

# Executable
add_executable(aurora4x ${SOURCES})

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
