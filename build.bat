@echo off
REM Build script for Aurora 4X-Like Space Empire Game (Windows)

echo ============================================
echo Building Aurora 4X-Like...
echo ============================================

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Run CMake configuration
echo Configuring with CMake...
cmake ..
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building the project...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo Build successful!
echo ============================================
echo Executable location: build\Release\aurora4x.exe
echo.
echo To run the game:
echo   build\Release\aurora4x.exe
echo.
echo Or from the build directory:
echo   cd build\Release ^&^& aurora4x.exe
echo ============================================
pause
