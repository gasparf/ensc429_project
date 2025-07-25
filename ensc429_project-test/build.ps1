#!/usr/bin/env powershell
# ENSC 429 Real-Time Vocoder - First-Time Build Script
# This script performs complete setup and build for new users

param(
    [switch]$SkipSetup,
    [switch]$CleanBuild,
    [switch]$Release,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$ColorSuccess = "Green"
$ColorWarning = "Yellow"
$ColorError = "Red"
$ColorInfo = "Cyan"

function Write-ColorOutput {
    param($Message, $Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Test-Prerequisites {
    Write-ColorOutput "`n=== Checking Prerequisites ===" $ColorInfo
    
    $allGood = $true
    
    # Check CMake
    try {
        $cmakeVersion = cmake --version 2>$null
        if ($cmakeVersion) {
            Write-ColorOutput "✓ CMake: $($cmakeVersion[0])" $ColorSuccess
        } else {
            Write-ColorOutput "✗ CMake not found" $ColorError
            $allGood = $false
        }
    } catch {
        Write-ColorOutput "✗ CMake not found" $ColorError
        $allGood = $false
    }
    
    # Check Git
    try {
        $gitVersion = git --version 2>$null
        if ($gitVersion) {
            Write-ColorOutput "✓ Git: $gitVersion" $ColorSuccess
        } else {
            Write-ColorOutput "✗ Git not found" $ColorError
            $allGood = $false
        }
    } catch {
        Write-ColorOutput "✗ Git not found" $ColorError
        $allGood = $false
    }
    
    # Check Visual Studio
    $vsFound = $false
    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat"
    )
    
    foreach ($path in $vsPaths) {
        $found = Get-ChildItem -Path $path -ErrorAction SilentlyContinue
        if ($found) {
            Write-ColorOutput "✓ Visual Studio: Found at $($found[0].DirectoryName)" $ColorSuccess
            $vsFound = $true
            break
        }
    }
    
    if (-not $vsFound) {
        Write-ColorOutput "✗ Visual Studio Build Tools not found" $ColorError
        $allGood = $false
    }
    
    return $allGood
}

function Initialize-Project {
    Write-ColorOutput "`n=== Initializing Project ===" $ColorInfo
    
    # Check if we're in a git repository
    try {
        git rev-parse --git-dir | Out-Null
        Write-ColorOutput "✓ Git repository detected" $ColorSuccess
        
        # Initialize submodules
        Write-ColorOutput "Initializing PortAudio submodule..." $ColorInfo
        git submodule update --init --recursive
        
        if (Test-Path "portaudio\include\portaudio.h") {
            Write-ColorOutput "✓ PortAudio submodule initialized" $ColorSuccess
        } else {
            Write-ColorOutput "✗ PortAudio submodule initialization failed" $ColorError
            return $false
        }
    } catch {
        Write-ColorOutput "⚠ Not in a git repository - assuming PortAudio is present" $ColorWarning
        
        if (-not (Test-Path "portaudio\include\portaudio.h")) {
            Write-ColorOutput "✗ PortAudio not found and not in git repository" $ColorError
            Write-ColorOutput "Please ensure PortAudio is available or run from a git repository" $ColorError
            return $false
        }
    }
    
    # Verify project structure
    $requiredFiles = @(
        "src\main.cpp",
        "src\dsp\dsp.hpp",
        "src\dsp\dsp.cpp",
        "CMakeLists.txt"
    )
    
    $missingFiles = @()
    foreach ($file in $requiredFiles) {
        if (-not (Test-Path $file)) {
            $missingFiles += $file
        }
    }
    
    if ($missingFiles.Count -gt 0) {
        Write-ColorOutput "✗ Missing required files:" $ColorError
        foreach ($file in $missingFiles) {
            Write-ColorOutput "  - $file" $ColorError
        }
        return $false
    }
    
    Write-ColorOutput "✓ Project structure verified" $ColorSuccess
    return $true
}

function Build-Project {
    param($BuildType = "Debug")
    
    Write-ColorOutput "`n=== Building Project ($BuildType) ===" $ColorInfo
    
    # Create build directory
    $buildDir = "build"
    if ($CleanBuild -and (Test-Path $buildDir)) {
        Write-ColorOutput "Cleaning previous build..." $ColorInfo
        Remove-Item $buildDir -Recurse -Force
    }
    
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
        Write-ColorOutput "✓ Build directory created" $ColorSuccess
    }
    
    # Change to build directory
    $originalLocation = Get-Location
    Set-Location $buildDir
    
    try {
        # Configure with CMake
        Write-ColorOutput "Configuring with CMake..." $ColorInfo
        $cmakeArgs = @("..")
        if ($BuildType -eq "Release") {
            $cmakeArgs += "-DCMAKE_BUILD_TYPE=Release"
        }
        
        if ($Verbose) {
            $cmakeOutput = & cmake @cmakeArgs
        } else {
            $cmakeOutput = & cmake @cmakeArgs 2>&1
        }
        
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput "✗ CMake configuration failed:" $ColorError
            Write-ColorOutput $cmakeOutput $ColorError
            return $false
        }
        
        Write-ColorOutput "✓ CMake configuration successful" $ColorSuccess
        
        # Build the project
        Write-ColorOutput "Building project..." $ColorInfo
        $buildArgs = @("--build", ".", "--config", $BuildType)
        
        if ($env:NUMBER_OF_PROCESSORS) {
            $buildArgs += "--parallel"
            $buildArgs += $env:NUMBER_OF_PROCESSORS
        }
        
        if ($Verbose) {
            $buildOutput = & cmake @buildArgs
        } else {
            $buildOutput = & cmake @buildArgs 2>&1
        }
        
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput "✗ Build failed:" $ColorError
            Write-ColorOutput $buildOutput $ColorError
            return $false
        }
        
        Write-ColorOutput "✓ Build successful" $ColorSuccess
        
        # Verify executable exists
        $exePath = if ($BuildType -eq "Debug") { "bin\voice_encoder_debug.exe" } else { "bin\voice_encoder.exe" }
        if (-not (Test-Path $exePath)) {
            $exePath = if ($BuildType -eq "Debug") { "Debug\voice_encoder.exe" } else { "Release\voice_encoder.exe" }
        }
        
        if (Test-Path $exePath) {
            $fullExePath = Resolve-Path $exePath
            Write-ColorOutput "✓ Executable created: $fullExePath" $ColorSuccess
            return $fullExePath
        } else {
            Write-ColorOutput "⚠ Executable not found at expected location" $ColorWarning
            return $true
        }
        
    } finally {
        Set-Location $originalLocation
    }
}

function Test-Application {
    param($ExePath)
    
    if (-not $ExePath -or -not (Test-Path $ExePath)) {
        Write-ColorOutput "⚠ Cannot test application - executable not found" $ColorWarning
        return
    }
    
    Write-ColorOutput "`n=== Testing Application ===" $ColorInfo
    
    try {
        # Quick test run (will likely fail due to audio, but checks if it loads)
        Write-ColorOutput "Running quick application test..." $ColorInfo
        $testProcess = Start-Process -FilePath $ExePath -ArgumentList "--help" -PassThru -NoNewWindow -RedirectStandardOutput "test_output.txt" -RedirectStandardError "test_error.txt"
        
        if ($testProcess.WaitForExit(5000)) {  # 5 second timeout
            if ($testProcess.ExitCode -eq 0) {
                Write-ColorOutput "✓ Application test passed" $ColorSuccess
            } else {
                Write-ColorOutput "⚠ Application exited with code $($testProcess.ExitCode)" $ColorWarning
                Write-ColorOutput "This may be normal if no audio devices are available" $ColorWarning
            }
        } else {
            Write-ColorOutput "⚠ Application test timed out" $ColorWarning
            $testProcess.Kill()
        }
        
        # Clean up test files
        if (Test-Path "test_output.txt") { Remove-Item "test_output.txt" -Force }
        if (Test-Path "test_error.txt") { Remove-Item "test_error.txt" -Force }
        
    } catch {
        Write-ColorOutput "⚠ Application test failed: $_" $ColorWarning
    }
}

function Show-Summary {
    param($Success, $ExePath, $BuildType)
    
    Write-ColorOutput "`n" + "="*60 $ColorInfo
    Write-ColorOutput "BUILD SUMMARY" $ColorInfo
    Write-ColorOutput "="*60 $ColorInfo
    
    if ($Success) {
        Write-ColorOutput "🎉 Build completed successfully!" $ColorSuccess
        Write-ColorOutput "" 
        Write-ColorOutput "Project: ENSC 429 Real-Time Vocoder" $ColorInfo
        Write-ColorOutput "Build Type: $BuildType" $ColorInfo
        if ($ExePath) {
            Write-ColorOutput "Executable: $ExePath" $ColorInfo
        }
        Write-ColorOutput ""
        Write-ColorOutput "To run the application:" $ColorInfo
        if ($ExePath) {
            $relativePath = Resolve-Path $ExePath -Relative
            Write-ColorOutput "  $relativePath" $ColorSuccess
        } else {
            Write-ColorOutput "  .\build\bin\voice_encoder.exe" $ColorSuccess
        }
        Write-ColorOutput ""
        Write-ColorOutput "Next steps:" $ColorInfo
        Write-ColorOutput "1. Connect a microphone and speakers/headphones" $ColorInfo
        Write-ColorOutput "2. Run the application to test real-time audio processing" $ColorInfo
        Write-ColorOutput "3. Check the DSP effects are working correctly" $ColorInfo
        
    } else {
        Write-ColorOutput "❌ Build failed!" $ColorError
        Write-ColorOutput ""
        Write-ColorOutput "Common solutions:" $ColorInfo
        Write-ColorOutput "1. Run the setup script: .\setup.ps1" $ColorInfo
        Write-ColorOutput "2. Install missing dependencies manually" $ColorInfo
        Write-ColorOutput "3. Run diagnostics: .\diagnose.ps1" $ColorInfo
        Write-ColorOutput "4. Check the build log for specific errors" $ColorInfo
    }
    
    Write-ColorOutput "="*60 $ColorInfo
}

# Main execution
Write-ColorOutput "ENSC 429 Real-Time Vocoder - First-Time Build" $ColorInfo
Write-ColorOutput "Starting build process at $(Get-Date)" $ColorInfo

# Run setup if not skipped
if (-not $SkipSetup) {
    if (-not (Test-Prerequisites)) {
        Write-ColorOutput "Prerequisites not met. Running setup..." $ColorWarning
        
        if (Test-Path "setup.ps1") {
            & ".\setup.ps1" -SkipConfirmation
            if ($LASTEXITCODE -ne 0) {
                Write-ColorOutput "Setup failed. Please check setup.log for details." $ColorError
                exit 1
            }
        } else {
            Write-ColorOutput "Setup script not found. Please run prerequisites check manually." $ColorError
            exit 1
        }
    }
}

# Initialize project
if (-not (Initialize-Project)) {
    Write-ColorOutput "Project initialization failed." $ColorError
    exit 1
}

# Build project
$buildType = if ($Release) { "Release" } else { "Debug" }
$buildResult = Build-Project -BuildType $buildType

if ($buildResult -eq $false) {
    Show-Summary -Success $false -BuildType $buildType
    exit 1
}

# Test application if executable was found
if ($buildResult -is [string]) {
    Test-Application -ExePath $buildResult
}

# Show summary
Show-Summary -Success $true -ExePath $buildResult -BuildType $buildType

Write-ColorOutput "`nBuild completed successfully at $(Get-Date)" $ColorSuccess
