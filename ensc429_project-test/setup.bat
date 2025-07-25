@echo off
REM ENSC 429 Real-Time Vocoder - Automated Setup Script (Batch Version)
REM This is a fallback script for systems where PowerShell execution is restricted

echo ========================================
echo ENSC 429 Real-Time Vocoder Setup
echo ========================================
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Warning: Not running as administrator. Some installations may fail.
    echo Please right-click and select "Run as administrator" for best results.
    echo.
    pause
)

echo Checking system requirements...

REM Check Windows version
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
if "%version%" lss "10.0" (
    echo Error: Windows 10 or newer is required.
    pause
    exit /b 1
)
echo - Windows version check passed

REM Check for winget
echo.
echo Checking for Windows Package Manager...
winget --version >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing Windows Package Manager...
    echo Please install the App Installer from Microsoft Store or visit:
    echo https://aka.ms/getwinget
    echo.
    echo After installation, please run this script again.
    pause
    exit /b 1
) else (
    echo - Windows Package Manager found
)

REM Check for CMake
echo.
echo Checking for CMake...
cmake --version >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing CMake...
    winget install --id Kitware.CMake --silent --accept-package-agreements --accept-source-agreements
    if %errorLevel% neq 0 (
        echo Failed to install CMake. Please install manually from:
        echo https://cmake.org/download/
        pause
        exit /b 1
    )
    echo - CMake installed successfully
    
    REM Refresh PATH
    call refreshenv
) else (
    echo - CMake found
)

REM Check for Git
echo.
echo Checking for Git...
git --version >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing Git...
    winget install --id Git.Git --silent --accept-package-agreements --accept-source-agreements
    if %errorLevel% neq 0 (
        echo Failed to install Git. Please install manually from:
        echo https://git-scm.com/download/win
        pause
        exit /b 1
    )
    echo - Git installed successfully
    
    REM Refresh PATH
    call refreshenv
) else (
    echo - Git found
)

REM Check for Visual Studio Build Tools
echo.
echo Checking for Visual Studio Build Tools...
set "VS_FOUND=0"

REM Check for VS 2022
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"

REM Check for VS 2019
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" set "VS_FOUND=1"

if "%VS_FOUND%"=="1" (
    echo - Visual Studio Build Tools found
) else (
    echo Installing Visual Studio Build Tools 2022...
    winget install --id Microsoft.VisualStudio.2022.BuildTools --silent --accept-package-agreements --accept-source-agreements --override "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools"
    if %errorLevel% neq 0 (
        echo Failed to install Visual Studio Build Tools.
        echo Please install manually from:
        echo https://visualstudio.microsoft.com/downloads/
        echo.
        echo Select "Build Tools for Visual Studio 2022" and include:
        echo - C++ build tools
        echo - Windows 10/11 SDK
        pause
        exit /b 1
    )
    echo - Visual Studio Build Tools installed successfully
    echo Note: You may need to restart your computer for changes to take effect.
)

REM Initialize PortAudio submodule
echo.
echo Initializing PortAudio submodule...
git rev-parse --git-dir >nul 2>&1
if %errorLevel% equ 0 (
    git submodule init
    git submodule update --recursive
    if %errorLevel% equ 0 (
        echo - PortAudio submodule initialized successfully
    ) else (
        echo Warning: Failed to initialize PortAudio submodule
        echo This may be normal if PortAudio is already present
    )
) else (
    echo Warning: Not in a git repository - skipping submodule initialization
)

REM Test build configuration
echo.
echo Testing build configuration...
if exist "build" rmdir /s /q "build"
mkdir build
cd build

cmake .. >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: CMake configuration failed
    echo Please check that all dependencies are properly installed
    cd ..
    pause
    exit /b 1
)

echo - CMake configuration successful
cd ..

echo.
echo ========================================
echo Setup completed successfully!
echo ========================================
echo.
echo You can now build the project with:
echo   mkdir build
echo   cd build
echo   cmake ..
echo   cmake --build .
echo.
echo The executable will be in: build\Debug\voice_encoder.exe
echo.
pause
