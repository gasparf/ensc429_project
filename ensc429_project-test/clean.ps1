#!/usr/bin/env powershell
# ENSC 429 Real-Time Vocoder - Clean Build Script (PowerShell)
# This script removes all build artifacts and temporary files

param(
    [switch]$Verbose
)

$ColorSuccess = "Green"
$ColorInfo = "Cyan"
$ColorWarning = "Yellow"

function Write-ColorOutput {
    param($Message, $Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

Write-ColorOutput "ENSC 429 Real-Time Vocoder - Clean Build" $ColorInfo
Write-ColorOutput "=" * 50 $ColorInfo
Write-ColorOutput ""

# Remove build directory
if (Test-Path "build") {
    Write-ColorOutput "Removing build/ directory..." $ColorInfo
    Remove-Item "build" -Recurse -Force
    Write-ColorOutput "✓ build/ directory removed" $ColorSuccess
} else {
    Write-ColorOutput "ℹ build/ directory not found" $ColorWarning
}

# Remove compiled artifacts
Write-ColorOutput "Cleaning compiled artifacts..." $ColorInfo
$artifacts = Get-ChildItem -Path "." -Recurse -Include "*.exe","*.dll","*.lib","*.obj","*.pdb","*.ilk","*.exp" -ErrorAction SilentlyContinue
if ($artifacts) {
    $artifacts | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-ColorOutput "✓ Compiled artifacts cleaned ($($artifacts.Count) files)" $ColorSuccess
} else {
    Write-ColorOutput "ℹ No compiled artifacts found" $ColorWarning
}

# Remove CMake files
Write-ColorOutput "Cleaning CMake files..." $ColorInfo
$cmakeFiles = Get-ChildItem -Path "." -Recurse -Include "CMakeCache.txt","cmake_install.cmake" -ErrorAction SilentlyContinue
if ($cmakeFiles) {
    $cmakeFiles | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-ColorOutput "✓ CMake files cleaned ($($cmakeFiles.Count) files)" $ColorSuccess
}

# Remove CMakeFiles directories
$cmakeDirs = Get-ChildItem -Path "." -Recurse -Directory -Name "CMakeFiles" -ErrorAction SilentlyContinue
if ($cmakeDirs) {
    $cmakeDirs | ForEach-Object { Remove-Item $_ -Recurse -Force -ErrorAction SilentlyContinue }
    Write-ColorOutput "✓ CMakeFiles directories cleaned ($($cmakeDirs.Count) directories)" $ColorSuccess
}

# Remove Visual Studio files
Write-ColorOutput "Cleaning Visual Studio files..." $ColorInfo
$vsFiles = Get-ChildItem -Path "." -Recurse -Include "*.vcxproj","*.vcxproj.filters","*.sln" -ErrorAction SilentlyContinue
if ($vsFiles) {
    $vsFiles | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-ColorOutput "✓ Visual Studio files cleaned ($($vsFiles.Count) files)" $ColorSuccess
}

# Remove log files
Write-ColorOutput "Cleaning log files..." $ColorInfo
$logFiles = Get-ChildItem -Path "." -Recurse -Include "*.log","setup.log","test_output.txt","test_error.txt" -ErrorAction SilentlyContinue
if ($logFiles) {
    $logFiles | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-ColorOutput "✓ Log files cleaned ($($logFiles.Count) files)" $ColorSuccess
}

Write-ColorOutput ""
Write-ColorOutput "🧹 Clean completed successfully!" $ColorSuccess
Write-ColorOutput ""
Write-ColorOutput "Your project is now clean and ready for a fresh build." $ColorInfo
Write-ColorOutput "Run '.\build.bat' or '.\build.ps1' to rebuild the project." $ColorInfo
