cmake_minimum_required(VERSION 3.10)
project(Aurora4XLike VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    target_compile_options(aurora4x PRIVATE /W4)
else()
    target_compile_options(aurora4x PRIVATE -Wall -Wextra -pedantic)
endif()
