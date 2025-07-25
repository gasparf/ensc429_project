#!/usr/bin/env powershell
# ENSC 429 Real-Time Vocoder - Automated Setup Script
# This script will install all required dependencies for first-time users

param(
    [switch]$SkipConfirmation,
    [switch]$Verbose,
    [string]$InstallPath = $null
)

# Script configuration
$ErrorActionPreference = "Stop"
$ProgressPreference = "Continue"

# Colors for output
$ColorSuccess = "Green"
$ColorWarning = "Yellow"
$ColorError = "Red"
$ColorInfo = "Cyan"

# Log file setup
$LogFile = Join-Path $PWD "setup.log"
$StartTime = Get-Date

function Write-Log {
    param($Message, $Level = "INFO", $Color = "White")
    
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $LogEntry = "[$Timestamp] [$Level] $Message"
    
    # Write to console with color
    Write-Host $Message -ForegroundColor $Color
    
    # Write to log file
    Add-Content -Path $LogFile -Value $LogEntry
}

function Test-AdminRights {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-SystemRequirements {
    Write-Log "Checking system requirements..." "INFO" $ColorInfo
    
    # Check Windows version
    $osVersion = [System.Environment]::OSVersion.Version
    if ($osVersion.Major -lt 10) {
        Write-Log "Error: Windows 10 or newer is required. Found: $($osVersion)" "ERROR" $ColorError
        return $false
    }
    Write-Log "✓ Windows version check passed" "INFO" $ColorSuccess
    
    # Check architecture
    if ([System.Environment]::Is64BitOperatingSystem -eq $false) {
        Write-Log "Error: 64-bit Windows is required" "ERROR" $ColorError
        return $false
    }
    Write-Log "✓ 64-bit architecture confirmed" "INFO" $ColorSuccess
    
    # Check available disk space (2GB minimum)
    $systemDrive = Get-WmiObject -Class Win32_LogicalDisk | Where-Object { $_.DeviceID -eq $env:SystemDrive }
    $freeSpaceGB = [math]::Round($systemDrive.FreeSpace / 1GB, 2)
    if ($freeSpaceGB -lt 2) {
        Write-Log "Warning: Low disk space. Available: ${freeSpaceGB}GB, Recommended: 2GB" "WARNING" $ColorWarning
    } else {
        Write-Log "✓ Sufficient disk space available: ${freeSpaceGB}GB" "INFO" $ColorSuccess
    }
    
    return $true
}

function Install-Winget {
    Write-Log "Checking for Windows Package Manager (winget)..." "INFO" $ColorInfo
    
    try {
        $wingetVersion = winget --version
        Write-Log "✓ Winget found: $wingetVersion" "INFO" $ColorSuccess
        return $true
    } catch {
        Write-Log "Installing Windows Package Manager..." "INFO" $ColorInfo
        
        try {
            # Download and install App Installer which includes winget
            $appInstallerUrl = "https://aka.ms/getwinget"
            $tempFile = "$env:TEMP\AppInstaller.msixbundle"
            
            Invoke-WebRequest -Uri $appInstallerUrl -OutFile $tempFile
            Add-AppxPackage -Path $tempFile
            
            Remove-Item $tempFile -Force
            Write-Log "✓ Windows Package Manager installed successfully" "INFO" $ColorSuccess
            return $true
        } catch {
            Write-Log "Failed to install winget: $_" "ERROR" $ColorError
            return $false
        }
    }
}

function Install-CMake {
    Write-Log "Checking for CMake..." "INFO" $ColorInfo
    
    try {
        $cmakeVersion = cmake --version
        Write-Log "✓ CMake found: $($cmakeVersion.Split([Environment]::NewLine)[0])" "INFO" $ColorSuccess
        return $true
    } catch {
        Write-Log "Installing CMake..." "INFO" $ColorInfo
        
        try {
            winget install --id Kitware.CMake --silent --accept-package-agreements --accept-source-agreements
            
            # Refresh PATH
            $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::Machine) + ";" + [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::User)
            
            # Verify installation
            Start-Sleep -Seconds 5
            $cmakeVersion = cmake --version
            Write-Log "✓ CMake installed successfully: $($cmakeVersion.Split([Environment]::NewLine)[0])" "INFO" $ColorSuccess
            return $true
        } catch {
            Write-Log "Failed to install CMake: $_" "ERROR" $ColorError
            return $false
        }
    }
}

function Install-Git {
    Write-Log "Checking for Git..." "INFO" $ColorInfo
    
    try {
        $gitVersion = git --version
        Write-Log "✓ Git found: $gitVersion" "INFO" $ColorSuccess
        return $true
    } catch {
        Write-Log "Installing Git..." "INFO" $ColorInfo
        
        try {
            winget install --id Git.Git --silent --accept-package-agreements --accept-source-agreements
            
            # Refresh PATH
            $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::Machine) + ";" + [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::User)
            
            Start-Sleep -Seconds 5
            $gitVersion = git --version
            Write-Log "✓ Git installed successfully: $gitVersion" "INFO" $ColorSuccess
            return $true
        } catch {
            Write-Log "Failed to install Git: $_" "ERROR" $ColorError
            return $false
        }
    }
}

function Install-VisualStudio {
    Write-Log "Checking for Visual Studio or Build Tools..." "INFO" $ColorInfo
    
    # Check for Visual Studio installations
    $vsInstallations = @()
    
    # Check common VS installation paths
    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles}\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\*\Common7\Tools\VsDevCmd.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\*\Common7\Tools\VsDevCmd.bat"
    )
    
    foreach ($path in $vsPaths) {
        $found = Get-ChildItem -Path $path -ErrorAction SilentlyContinue
        if ($found) {
            $vsInstallations += $found
        }
    }
    
    if ($vsInstallations.Count -gt 0) {
        Write-Log "✓ Visual Studio found: $($vsInstallations[0].DirectoryName)" "INFO" $ColorSuccess
        return $true
    }
    
    Write-Log "Installing Visual Studio Build Tools 2022..." "INFO" $ColorInfo
    
    try {
        # Install VS Build Tools with C++ workload
        winget install --id Microsoft.VisualStudio.2022.BuildTools --silent --accept-package-agreements --accept-source-agreements --override "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools"
        
        Write-Log "✓ Visual Studio Build Tools installed successfully" "INFO" $ColorSuccess
        Write-Log "Note: You may need to restart your computer for changes to take effect" "INFO" $ColorWarning
        return $true
    } catch {
        Write-Log "Failed to install Visual Studio Build Tools: $_" "ERROR" $ColorError
        Write-Log "Please install Visual Studio manually from: https://visualstudio.microsoft.com/downloads/" "INFO" $ColorInfo
        return $false
    }
}

function Initialize-PortAudio {
    Write-Log "Initializing PortAudio submodule..." "INFO" $ColorInfo
    
    try {
        # Check if we're in a git repository
        git rev-parse --git-dir | Out-Null
        
        # Initialize and update submodules
        git submodule init
        git submodule update --recursive
        
        # Verify PortAudio directory exists and has content
        $portAudioPath = Join-Path $PWD "portaudio"
        if (Test-Path $portAudioPath) {
            $files = Get-ChildItem -Path $portAudioPath
            if ($files.Count -gt 0) {
                Write-Log "✓ PortAudio submodule initialized successfully" "INFO" $ColorSuccess
                return $true
            }
        }
        
        Write-Log "Error: PortAudio submodule appears to be empty" "ERROR" $ColorError
        return $false
    } catch {
        Write-Log "Failed to initialize PortAudio submodule: $_" "ERROR" $ColorError
        Write-Log "Note: This may be normal if not in a git repository" "INFO" $ColorWarning
        return $false
    }
}

function Test-Build {
    Write-Log "Testing build configuration..." "INFO" $ColorInfo
    
    try {
        # Create build directory
        $buildDir = Join-Path $PWD "build"
        if (Test-Path $buildDir) {
            Remove-Item $buildDir -Recurse -Force
        }
        New-Item -ItemType Directory -Path $buildDir | Out-Null
        
        # Run CMake configuration
        Set-Location $buildDir
        $cmakeOutput = cmake .. 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Log "✓ CMake configuration successful" "INFO" $ColorSuccess
            
            # Test build (just configuration, not full build)
            $buildOutput = cmake --build . --config Debug --target voice_encoder --parallel 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Log "✓ Test build successful" "INFO" $ColorSuccess
                Set-Location ..
                return $true
            } else {
                Write-Log "Build test failed: $buildOutput" "ERROR" $ColorError
                Set-Location ..
                return $false
            }
        } else {
            Write-Log "CMake configuration failed: $cmakeOutput" "ERROR" $ColorError
            Set-Location ..
            return $false
        }
    } catch {
        Write-Log "Build test failed with exception: $_" "ERROR" $ColorError
        Set-Location $PWD
        return $false
    }
}

function Show-Summary {
    param($Results)
    
    Write-Log "`n" + "="*60 "INFO" $ColorInfo
    Write-Log "SETUP SUMMARY" "INFO" $ColorInfo
    Write-Log "="*60 "INFO" $ColorInfo
    
    foreach ($result in $Results.GetEnumerator()) {
        $status = if ($result.Value) { "✓ PASS" } else { "✗ FAIL" }
        $color = if ($result.Value) { $ColorSuccess } else { $ColorError }
        Write-Log "$($result.Key): $status" "INFO" $color
    }
    
    $totalTime = (Get-Date) - $StartTime
    Write-Log "`nTotal setup time: $($totalTime.ToString('mm\:ss'))" "INFO" $ColorInfo
    
    if ($Results.Values -contains $false) {
        Write-Log "`nSome components failed to install. Check the log file for details: $LogFile" "WARNING" $ColorWarning
        Write-Log "You may need to install failed components manually." "INFO" $ColorInfo
    } else {
        Write-Log "`n🎉 Setup completed successfully!" "INFO" $ColorSuccess
        Write-Log "You can now build the project with:" "INFO" $ColorInfo
        Write-Log "  mkdir build" "INFO" $ColorInfo
        Write-Log "  cd build" "INFO" $ColorInfo
        Write-Log "  cmake .." "INFO" $ColorInfo
        Write-Log "  cmake --build ." "INFO" $ColorInfo
    }
}

# Main setup function
function Start-Setup {
    Write-Log "ENSC 429 Real-Time Vocoder - Automated Setup" "INFO" $ColorInfo
    Write-Log "Starting setup at $(Get-Date)" "INFO" $ColorInfo
    Write-Log "Log file: $LogFile" "INFO" $ColorInfo
    Write-Log ""
    
    # Check if running as administrator
    if (-not (Test-AdminRights)) {
        Write-Log "Warning: Not running as administrator. Some installations may fail." "WARNING" $ColorWarning
        if (-not $SkipConfirmation) {
            $response = Read-Host "Continue anyway? (y/N)"
            if ($response -ne "y" -and $response -ne "Y") {
                Write-Log "Setup cancelled by user" "INFO" $ColorInfo
                exit 1
            }
        }
    }
    
    # Initialize results tracking
    $results = [ordered]@{
        "System Requirements" = $false
        "Windows Package Manager" = $false
        "CMake" = $false
        "Git" = $false
        "Visual Studio/Build Tools" = $false
        "PortAudio Submodule" = $false
        "Build Test" = $false
    }
    
    # Run setup steps
    $results["System Requirements"] = Test-SystemRequirements
    $results["Windows Package Manager"] = Install-Winget
    $results["CMake"] = Install-CMake
    $results["Git"] = Install-Git
    $results["Visual Studio/Build Tools"] = Install-VisualStudio
    $results["PortAudio Submodule"] = Initialize-PortAudio
    $results["Build Test"] = Test-Build
    
    # Show summary
    Show-Summary -Results $results
    
    # Return overall success
    return -not ($results.Values -contains $false)
}

# Script entry point
try {
    $success = Start-Setup
    exit $(if ($success) { 0 } else { 1 })
} catch {
    Write-Log "Setup failed with unexpected error: $_" "ERROR" $ColorError
    Write-Log "Stack trace: $($_.ScriptStackTrace)" "ERROR" $ColorError
    exit 1
}
