@echo off
REM ENSC 429 Real-Time Vocoder - Clean Build Script
REM This script removes all build artifacts and temporary files

echo ========================================
echo ENSC 429 Real-Time Vocoder - Clean
echo ========================================
echo.

echo Cleaning build artifacts...

REM Remove build directory
if exist "build" (
    echo Removing build/ directory...
    rmdir /s /q "build"
    echo [OK] build/ directory removed
) else (
    echo [INFO] build/ directory not found
)

REM Remove compiled binaries
echo Cleaning compiled artifacts...
for /r %%i in (*.exe *.dll *.lib *.obj *.pdb *.ilk *.exp) do (
    if exist "%%i" (
        del /f /q "%%i" 2>nul
    )
)

REM Remove CMake files
for /r %%i in (CMakeCache.txt cmake_install.cmake) do (
    if exist "%%i" (
        del /f /q "%%i" 2>nul
    )
)

REM Remove CMakeFiles directories
for /f "delims=" %%i in ('dir /s /b /ad CMakeFiles 2^>nul') do (
    rmdir /s /q "%%i" 2>nul
)

REM Remove Visual Studio files
for /r %%i in (*.vcxproj *.vcxproj.filters *.sln) do (
    if exist "%%i" (
        del /f /q "%%i" 2>nul
    )
)

REM Remove log files
for /r %%i in (*.log setup.log test_output.txt test_error.txt) do (
    if exist "%%i" (
        del /f /q "%%i" 2>nul
    )
)

echo [OK] Compiled artifacts cleaned
echo [OK] CMake files cleaned
echo [OK] Visual Studio files cleaned
echo [OK] Log files cleaned

echo.
echo ========================================
echo Clean completed successfully!
echo ========================================
echo.
echo Your project is now clean and ready for a fresh build.
echo Run ".\build.bat" to rebuild the project.
echo.
pause
