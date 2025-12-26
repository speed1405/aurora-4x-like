#!/bin/bash
# Build script for Aurora 4X-Like Space Empire Game

set -e

echo "============================================"
echo "Building Aurora 4X-Like..."
echo "============================================"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Run CMake configuration
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building the project..."
make -j$(nproc)

echo ""
echo "============================================"
echo "Build successful!"
echo "============================================"
echo "Executable location: build/aurora4x"
echo ""
echo "To run the game:"
echo "  ./build/aurora4x"
echo ""
echo "Or from the build directory:"
echo "  cd build && ./aurora4x"
echo "============================================"
