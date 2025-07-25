@echo off
REM ENSC 429 Real-Time Vocoder - First-Time Build Script (Batch Version)
REM This script performs complete setup and build for new users

setlocal EnableDelayedExpansion

set "BUILD_TYPE=Debug"
set "CLEAN_BUILD=0"
set "SKIP_SETUP=0"

REM Parse command line arguments
:parse_args
if "%1"=="/release" (
    set "BUILD_TYPE=Release"
    shift
    goto parse_args
)
if "%1"=="/clean" (
    set "CLEAN_BUILD=1"
    shift
    goto parse_args
)
if "%1"=="/skip-setup" (
    set "SKIP_SETUP=1"
    shift
    goto parse_args
)
if "%1"=="/?" goto show_help
if "%1"=="-h" goto show_help
if "%1"=="--help" goto show_help
if not "%1"=="" (
    shift
    goto parse_args
)

echo ========================================
echo ENSC 429 Real-Time Vocoder - Build
echo ========================================
echo Build Type: %BUILD_TYPE%
echo Clean Build: %CLEAN_BUILD%
echo.

REM Check prerequisites
echo === Checking Prerequisites ===
set "PREREQ_OK=1"

REM Check CMake
cmake --version >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] CMake not found
    set "PREREQ_OK=0"
) else (
    echo [OK] CMake found
)

REM Check Git
git --version >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Git not found
    set "PREREQ_OK=0"
) else (
    echo [OK] Git found
)

REM Check Visual Studio
set "VS_FOUND=0"
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles%\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"

if "%VS_FOUND%"=="1" (
    echo [OK] Visual Studio Build Tools found
) else (
    echo [ERROR] Visual Studio Build Tools not found
    set "PREREQ_OK=0"
)

REM Run setup if prerequisites are missing and not skipped
if "%PREREQ_OK%"=="0" (
    if "%SKIP_SETUP%"=="0" (
        echo.
        echo Prerequisites not met. Running setup...
        if exist "setup.bat" (
            call setup.bat
            if !errorLevel! neq 0 (
                echo Setup failed. Please check for errors.
                pause
                exit /b 1
            )
        ) else (
            echo Setup script not found. Please install prerequisites manually.
            echo.
            echo Required:
            echo - CMake: https://cmake.org/download/
            echo - Git: https://git-scm.com/download/win
            echo - Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/
            pause
            exit /b 1
        )
    ) else (
        echo Prerequisites not met and setup skipped. Cannot continue.
        pause
        exit /b 1
    )
)

echo.
echo === Initializing Project ===

REM Check if in git repository and initialize submodules
git rev-parse --git-dir >nul 2>&1
if %errorLevel% equ 0 (
    echo [INFO] Git repository detected
    echo Initializing PortAudio submodule...
    git submodule update --init --recursive
    if !errorLevel! neq 0 (
        echo [WARNING] Submodule initialization failed
    ) else (
        echo [OK] PortAudio submodule initialized
    )
) else (
    echo [WARNING] Not in a git repository - assuming PortAudio is present
)

REM Verify PortAudio
if exist "portaudio\include\portaudio.h" (
    echo [OK] PortAudio found
) else (
    echo [ERROR] PortAudio not found
    echo Please ensure PortAudio is available or run from a git repository
    pause
    exit /b 1
)

REM Verify project structure
echo Verifying project structure...
set "STRUCTURE_OK=1"

if not exist "src\main.cpp" (
    echo [ERROR] src\main.cpp not found
    set "STRUCTURE_OK=0"
)
if not exist "src\dsp\dsp.hpp" (
    echo [ERROR] src\dsp\dsp.hpp not found
    set "STRUCTURE_OK=0"
)
if not exist "src\dsp\dsp.cpp" (
    echo [ERROR] src\dsp\dsp.cpp not found
    set "STRUCTURE_OK=0"
)
if not exist "CMakeLists.txt" (
    echo [ERROR] CMakeLists.txt not found
    set "STRUCTURE_OK=0"
)

if "%STRUCTURE_OK%"=="0" (
    echo Project structure verification failed
    pause
    exit /b 1
)

echo [OK] Project structure verified

echo.
echo === Building Project ===

REM Clean build if requested
if "%CLEAN_BUILD%"=="1" (
    if exist "build" (
        echo Cleaning previous build...
        rmdir /s /q "build"
    )
)

REM Create build directory
if not exist "build" (
    mkdir build
    echo [OK] Build directory created
)

REM Change to build directory
cd build

REM Configure with CMake
echo Configuring with CMake...
if "%BUILD_TYPE%"=="Release" (
    cmake .. -DCMAKE_BUILD_TYPE=Release
) else (
    cmake ..
)

if %errorLevel% neq 0 (
    echo [ERROR] CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo [OK] CMake configuration successful

REM Build the project
echo Building project...
cmake --build . --config %BUILD_TYPE% --parallel

if %errorLevel% neq 0 (
    echo [ERROR] Build failed
    cd ..
    pause
    exit /b 1
)

echo [OK] Build successful

REM Find the executable
set "EXE_PATH="
if "%BUILD_TYPE%"=="Debug" (
    if exist "bin\voice_encoder_debug.exe" set "EXE_PATH=bin\voice_encoder_debug.exe"
    if exist "Debug\voice_encoder.exe" set "EXE_PATH=Debug\voice_encoder.exe"
) else (
    if exist "bin\voice_encoder.exe" set "EXE_PATH=bin\voice_encoder.exe"
    if exist "Release\voice_encoder.exe" set "EXE_PATH=Release\voice_encoder.exe"
)

if not "%EXE_PATH%"=="" (
    echo [OK] Executable created: %EXE_PATH%
) else (
    echo [WARNING] Executable not found at expected location
)

cd ..

echo.
echo === Build Summary ===
echo ========================================
echo Build Type: %BUILD_TYPE%
echo Status: SUCCESS
if not "%EXE_PATH%"=="" (
    echo Executable: build\%EXE_PATH%
)
echo.
echo To run the application:
if not "%EXE_PATH%"=="" (
    echo   build\%EXE_PATH%
) else (
    echo   build\Debug\voice_encoder.exe  (or Release)
)
echo.
echo Next steps:
echo 1. Connect a microphone and speakers/headphones
echo 2. Run the application to test real-time audio processing
echo 3. Check the DSP effects are working correctly
echo ========================================

pause
goto :eof

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   /release     Build in Release mode (default: Debug)
echo   /clean       Clean previous build before building
echo   /skip-setup  Skip automatic setup if prerequisites are missing
echo   /?           Show this help message
echo.
echo Examples:
echo   build.bat                    Build in Debug mode
echo   build.bat /release           Build in Release mode
echo   build.bat /clean /release    Clean build in Release mode
echo.
pause
goto :eof
