@echo off
REM Build script for Aurora 4X-Like Space Empire Game (Windows)

REM Set AURORA_BUILD_DEBUG=1 to echo commands for troubleshooting.
if defined AURORA_BUILD_DEBUG echo on

setlocal enableextensions enabledelayedexpansion

echo ============================================
echo Building Aurora 4X-Like...
echo ============================================

REM Clean previous CMake cache to avoid generator/platform mismatches
if exist "build\CMakeCache.txt" (
    echo Cleaning previous build directory...
    rmdir /s /q build
)

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Find Visual Studio Build Tools / VS installation (for MSVC)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo ERROR: vswhere.exe not found. Install Visual Studio Build Tools 2022.
    echo        https://visualstudio.microsoft.com/downloads/
    if not defined AURORA_NO_PAUSE pause
    exit /b 1
)

set "VSINSTALL="
for /f "delims=" %%i in ('"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set "VSINSTALL=%%i"

if "!VSINSTALL!"=="" (
    echo ERROR: MSVC C++ toolset not found.
    echo Install the "Desktop development with C++" workload or the VC Tools component.
    if not defined AURORA_NO_PAUSE pause
    exit /b 1
)

set "VSDEVCMD=!VSINSTALL!\Common7\Tools\VsDevCmd.bat"
if not exist "!VSDEVCMD!" (
    echo ERROR: VsDevCmd.bat not found at: !VSDEVCMD!
    if not defined AURORA_NO_PAUSE pause
    exit /b 1
)

REM Bootstrap MSVC environment (x64)
call "!VSDEVCMD!" -arch=amd64 -host_arch=amd64
if errorlevel 1 (
    echo Failed to initialize MSVC environment!
    if not defined AURORA_NO_PAUSE pause
    exit /b 1
)

REM Run CMake configuration
echo Configuring with CMake...
cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_CXX_COMPILER=cl
if errorlevel 1 (
    echo CMake configuration failed!
    if not defined AURORA_NO_PAUSE pause
    exit /b 1
)

REM Build the project
echo Building the project...
cmake --build build --config Release
if errorlevel 1 (
    echo Build failed!
    if not defined AURORA_NO_PAUSE pause
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
if not defined AURORA_NO_PAUSE pause

endlocal
