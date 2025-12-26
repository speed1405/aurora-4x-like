# Build Instructions for Aurora 4X-Like

This document provides detailed instructions for building the Aurora 4X-Like executable from source.

## Prerequisites

### All Platforms
- **CMake** 3.10 or higher
- **C++17** compatible compiler

### Linux/Mac
- GCC 7+ or Clang 5+
- Make

### Windows
- Visual Studio 2017+ with C++ support
- Or MinGW with GCC 7+

## Quick Build

### Linux/Mac

The easiest way to build is using the provided build script:

```bash
./build.sh
```

This will:
1. Create a `build` directory
2. Configure the project with CMake
3. Compile the source code
4. Create the executable at `build/aurora4x`

### Windows

Use the provided batch file:

```batch
build.bat
```

This will:
1. Create a `build` directory
2. Configure the project with CMake
3. Compile the source code
4. Create the executable at `build\Release\aurora4x.exe`

## Manual Build

### Linux/Mac

```bash
# Create and enter build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build with make (use -j for parallel build)
make -j$(nproc)

# The executable is now at: aurora4x
```

### Windows (Visual Studio)

```batch
REM Create and enter build directory
mkdir build
cd build

REM Configure with CMake
cmake ..

REM Build with CMake
cmake --build . --config Release

REM The executable is now at: Release\aurora4x.exe
```

### Windows (MinGW)

```batch
REM Create and enter build directory
mkdir build
cd build

REM Configure with CMake for MinGW
cmake -G "MinGW Makefiles" ..

REM Build with mingw32-make
mingw32-make

REM The executable is now at: aurora4x.exe
```

## Running the Executable

### Linux/Mac

From the project root:
```bash
./build/aurora4x
```

Or from the build directory:
```bash
cd build
./aurora4x
```

### Windows

From the project root:
```batch
build\Release\aurora4x.exe
```

Or from the build directory:
```batch
cd build\Release
aurora4x.exe
```

## Build Options

### Debug Build

For a debug build with symbols:

**Linux/Mac:**
```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

**Windows:**
```batch
mkdir build-debug
cd build-debug
cmake ..
cmake --build . --config Debug
```

### Release Build with Optimizations

For an optimized release build:

**Linux/Mac:**
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

**Windows:**
```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Cleaning Build Artifacts

To clean and rebuild:

```bash
# Remove the build directory
rm -rf build

# Run the build script again
./build.sh  # Linux/Mac
build.bat   # Windows
```

## Troubleshooting

### CMake Not Found

Install CMake:
- **Linux (Ubuntu/Debian):** `sudo apt-get install cmake`
- **Linux (Fedora):** `sudo dnf install cmake`
- **Mac:** `brew install cmake`
- **Windows:** Download from https://cmake.org/download/

### Compiler Not Found

Install a C++ compiler:
- **Linux (Ubuntu/Debian):** `sudo apt-get install build-essential`
- **Linux (Fedora):** `sudo dnf install gcc-c++`
- **Mac:** `xcode-select --install`
- **Windows:** Install Visual Studio with C++ workload or MinGW

### Build Fails with C++17 Errors

Ensure your compiler supports C++17:
- GCC 7 or later
- Clang 5 or later
- Visual Studio 2017 or later

Update your compiler if necessary.

### Permission Denied on Linux/Mac

Make the build script executable:
```bash
chmod +x build.sh
```

## Binary Distribution

The built executable is standalone and can be distributed:

- **Linux/Mac:** The `aurora4x` executable
- **Windows:** The `aurora4x.exe` executable

Note: On Linux, users may need to have the C++ runtime libraries installed.

## Project Structure

```
aurora-4x-like/
├── build/              # Build directory (created during build, gitignored)
├── include/            # Header files
├── src/                # Source files
├── CMakeLists.txt      # CMake configuration
├── build.sh            # Linux/Mac build script
├── build.bat           # Windows build script
└── BUILD_INSTRUCTIONS.md  # This file
```

## Additional Information

For gameplay instructions, see [README.md](README.md).

For feature details, see [FEATURES.md](FEATURES.md).

For quick start guide, see [QUICKSTART.md](QUICKSTART.md).
