#!/usr/bin/env powershell
# ENSC 429 Real-Time Vocoder - System Diagnostic Script
# This script diagnoses common setup and build issues

param(
    [switch]$Verbose,
    [switch]$ExportReport
)

$ErrorActionPreference = "Continue"
$ColorSuccess = "Green"
$ColorWarning = "Yellow"
$ColorError = "Red"
$ColorInfo = "Cyan"

$DiagnosticResults = @()

function Add-DiagnosticResult {
    param($Category, $Test, $Status, $Details, $Recommendation = "")
    
    $result = [PSCustomObject]@{
        Category = $Category
        Test = $Test
        Status = $Status
        Details = $Details
        Recommendation = $Recommendation
        Timestamp = Get-Date
    }
    
    $script:DiagnosticResults += $result
    
    $color = switch ($Status) {
        "PASS" { $ColorSuccess }
        "FAIL" { $ColorError }
        "WARNING" { $ColorWarning }
        default { $ColorInfo }
    }
    
    Write-Host "[$Category] $Test : $Status" -ForegroundColor $color
    if ($Verbose -and $Details) {
        Write-Host "  Details: $Details" -ForegroundColor Gray
    }
    if ($Recommendation) {
        Write-Host "  Recommendation: $Recommendation" -ForegroundColor Yellow
    }
}

function Test-SystemEnvironment {
    Write-Host "`n=== System Environment ===" -ForegroundColor $ColorInfo
    
    # Windows Version
    $osVersion = [System.Environment]::OSVersion.Version
    $osName = (Get-WmiObject -Class Win32_OperatingSystem).Caption
    if ($osVersion.Major -ge 10) {
        Add-DiagnosticResult "System" "Windows Version" "PASS" "$osName (Version $osVersion)"
    } else {
        Add-DiagnosticResult "System" "Windows Version" "FAIL" "$osName (Version $osVersion)" "Upgrade to Windows 10 or newer"
    }
    
    # Architecture
    $is64Bit = [System.Environment]::Is64BitOperatingSystem
    if ($is64Bit) {
        Add-DiagnosticResult "System" "Architecture" "PASS" "64-bit"
    } else {
        Add-DiagnosticResult "System" "Architecture" "FAIL" "32-bit" "64-bit Windows is required"
    }
    
    # Memory
    $totalMemory = [math]::Round((Get-WmiObject -Class Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)
    if ($totalMemory -ge 4) {
        Add-DiagnosticResult "System" "Memory" "PASS" "${totalMemory}GB"
    } else {
        Add-DiagnosticResult "System" "Memory" "WARNING" "${totalMemory}GB" "4GB or more recommended"
    }
    
    # Disk Space
    $systemDrive = Get-WmiObject -Class Win32_LogicalDisk | Where-Object { $_.DeviceID -eq $env:SystemDrive }
    $freeSpaceGB = [math]::Round($systemDrive.FreeSpace / 1GB, 2)
    if ($freeSpaceGB -ge 2) {
        Add-DiagnosticResult "System" "Disk Space" "PASS" "${freeSpaceGB}GB free"
    } else {
        Add-DiagnosticResult "System" "Disk Space" "WARNING" "${freeSpaceGB}GB free" "2GB or more recommended"
    }
    
    # Administrator Rights
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    $isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if ($isAdmin) {
        Add-DiagnosticResult "System" "Admin Rights" "PASS" "Running as administrator"
    } else {
        Add-DiagnosticResult "System" "Admin Rights" "WARNING" "Not running as administrator" "Some installations may require admin rights"
    }
}

function Test-DevelopmentTools {
    Write-Host "`n=== Development Tools ===" -ForegroundColor $ColorInfo
    
    # CMake
    try {
        $cmakeVersion = (cmake --version 2>$null)[0]
        if ($cmakeVersion) {
            Add-DiagnosticResult "Tools" "CMake" "PASS" $cmakeVersion
        } else {
            Add-DiagnosticResult "Tools" "CMake" "FAIL" "Not found" "Install CMake from https://cmake.org"
        }
    } catch {
        Add-DiagnosticResult "Tools" "CMake" "FAIL" "Not found" "Install CMake from https://cmake.org"
    }
    
    # Git
    try {
        $gitVersion = git --version 2>$null
        if ($gitVersion) {
            Add-DiagnosticResult "Tools" "Git" "PASS" $gitVersion
        } else {
            Add-DiagnosticResult "Tools" "Git" "FAIL" "Not found" "Install Git from https://git-scm.com"
        }
    } catch {
        Add-DiagnosticResult "Tools" "Git" "FAIL" "Not found" "Install Git from https://git-scm.com"
    }
    
    # Windows Package Manager
    try {
        $wingetVersion = (winget --version 2>$null)
        if ($wingetVersion) {
            Add-DiagnosticResult "Tools" "WinGet" "PASS" $wingetVersion
        } else {
            Add-DiagnosticResult "Tools" "WinGet" "FAIL" "Not found" "Install from Microsoft Store or https://aka.ms/getwinget"
        }
    } catch {
        Add-DiagnosticResult "Tools" "WinGet" "FAIL" "Not found" "Install from Microsoft Store or https://aka.ms/getwinget"
    }
}

function Test-VisualStudio {
    Write-Host "`n=== Visual Studio / Build Tools ===" -ForegroundColor $ColorInfo
    
    # Find VS installations
    $vsInstallations = @()
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
        $vsPath = $vsInstallations[0].DirectoryName
        $vsVersion = if ($vsPath -match "2022") { "2022" } elseif ($vsPath -match "2019") { "2019" } else { "Unknown" }
        Add-DiagnosticResult "Build Tools" "Visual Studio" "PASS" "Found VS $vsVersion at $vsPath"
    } else {
        Add-DiagnosticResult "Build Tools" "Visual Studio" "FAIL" "Not found" "Install Visual Studio Build Tools"
    }
    
    # Test MSVC compiler availability
    try {
        # Try to find cl.exe in common locations
        $clPaths = @(
            "${env:ProgramFiles}\Microsoft Visual Studio\2022\*\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe",
            "${env:ProgramFiles}\Microsoft Visual Studio\2019\*\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe",
            "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\*\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe",
            "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\*\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe"
        )
        
        $clFound = $false
        foreach ($path in $clPaths) {
            $found = Get-ChildItem -Path $path -ErrorAction SilentlyContinue
            if ($found) {
                Add-DiagnosticResult "Build Tools" "MSVC Compiler" "PASS" "Found at $($found[0].DirectoryName)"
                $clFound = $true
                break
            }
        }
        
        if (-not $clFound) {
            Add-DiagnosticResult "Build Tools" "MSVC Compiler" "FAIL" "cl.exe not found" "Install C++ build tools workload"
        }
    } catch {
        Add-DiagnosticResult "Build Tools" "MSVC Compiler" "FAIL" "Error checking compiler" "Verify Visual Studio installation"
    }
    
    # Windows SDK
    $sdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
    if (Test-Path $sdkPath) {
        $sdkVersions = Get-ChildItem -Path "$sdkPath\Include" -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending
        if ($sdkVersions) {
            Add-DiagnosticResult "Build Tools" "Windows SDK" "PASS" "Found version $($sdkVersions[0].Name)"
        } else {
            Add-DiagnosticResult "Build Tools" "Windows SDK" "WARNING" "Directory exists but no versions found"
        }
    } else {
        Add-DiagnosticResult "Build Tools" "Windows SDK" "FAIL" "Not found" "Install Windows SDK"
    }
}

function Test-ProjectStructure {
    Write-Host "`n=== Project Structure ===" -ForegroundColor $ColorInfo
    
    # Check for required files
    $requiredFiles = @(
        "CMakeLists.txt",
        "src\main.cpp",
        "src\dsp\dsp.hpp",
        "src\dsp\dsp.cpp"
    )
    
    foreach ($file in $requiredFiles) {
        if (Test-Path $file) {
            Add-DiagnosticResult "Project" "File: $file" "PASS" "Found"
        } else {
            Add-DiagnosticResult "Project" "File: $file" "FAIL" "Missing" "Ensure project structure is complete"
        }
    }
    
    # Check PortAudio
    $portAudioPath = "portaudio"
    if (Test-Path $portAudioPath) {
        $portAudioFiles = Get-ChildItem -Path $portAudioPath -Recurse -File | Measure-Object
        if ($portAudioFiles.Count -gt 10) {
            Add-DiagnosticResult "Project" "PortAudio Submodule" "PASS" "$($portAudioFiles.Count) files found"
        } else {
            Add-DiagnosticResult "Project" "PortAudio Submodule" "WARNING" "Directory exists but appears empty" "Run 'git submodule update --init --recursive'"
        }
    } else {
        Add-DiagnosticResult "Project" "PortAudio Submodule" "FAIL" "Directory not found" "Initialize PortAudio submodule"
    }
    
    # Check build directory
    if (Test-Path "build") {
        $buildFiles = Get-ChildItem -Path "build" -File | Measure-Object
        if ($buildFiles.Count -gt 0) {
            Add-DiagnosticResult "Project" "Build Directory" "PASS" "Contains $($buildFiles.Count) files"
        } else {
            Add-DiagnosticResult "Project" "Build Directory" "WARNING" "Empty directory" "Run CMake to generate build files"
        }
    } else {
        Add-DiagnosticResult "Project" "Build Directory" "INFO" "Not present" "This is normal for first setup"
    }
}

function Test-AudioSystem {
    Write-Host "`n=== Audio System ===" -ForegroundColor $ColorInfo
    
    # Check audio devices
    try {
        Add-Type -TypeDefinition @"
            using System;
            using System.Runtime.InteropServices;
            public class WinMM {
                [DllImport("winmm.dll")]
                public static extern uint waveOutGetNumDevs();
                [DllImport("winmm.dll")]
                public static extern uint waveInGetNumDevs();
            }
"@
        
        $outputDevices = [WinMM]::waveOutGetNumDevs()
        $inputDevices = [WinMM]::waveInGetNumDevs()
        
        if ($outputDevices -gt 0) {
            Add-DiagnosticResult "Audio" "Output Devices" "PASS" "$outputDevices devices found"
        } else {
            Add-DiagnosticResult "Audio" "Output Devices" "FAIL" "No devices found" "Check audio drivers"
        }
        
        if ($inputDevices -gt 0) {
            Add-DiagnosticResult "Audio" "Input Devices" "PASS" "$inputDevices devices found"
        } else {
            Add-DiagnosticResult "Audio" "Input Devices" "FAIL" "No devices found" "Check microphone drivers"
        }
    } catch {
        Add-DiagnosticResult "Audio" "Device Detection" "WARNING" "Unable to check devices" "Audio system may not be properly configured"
    }
    
    # Check Windows Audio service
    $audioService = Get-Service -Name "AudioSrv" -ErrorAction SilentlyContinue
    if ($audioService) {
        if ($audioService.Status -eq "Running") {
            Add-DiagnosticResult "Audio" "Windows Audio Service" "PASS" "Running"
        } else {
            Add-DiagnosticResult "Audio" "Windows Audio Service" "FAIL" "Not running" "Start the Windows Audio service"
        }
    } else {
        Add-DiagnosticResult "Audio" "Windows Audio Service" "FAIL" "Not found" "Audio service is missing"
    }
}

function Test-BuildProcess {
    Write-Host "`n=== Build Process Test ===" -ForegroundColor $ColorInfo
    
    if (-not (Test-Path "CMakeLists.txt")) {
        Add-DiagnosticResult "Build" "CMakeLists.txt" "FAIL" "File not found" "Ensure you're in the project root directory"
        return
    }
    
    # Create temporary build directory for testing
    $tempBuildDir = "build_test_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
    
    try {
        New-Item -ItemType Directory -Path $tempBuildDir -Force | Out-Null
        Set-Location $tempBuildDir
        
        # Test CMake configuration
        $cmakeOutput = cmake .. 2>&1
        if ($LASTEXITCODE -eq 0) {
            Add-DiagnosticResult "Build" "CMake Configuration" "PASS" "Configuration successful"
            
            # Test build (just validate, don't complete)
            $buildOutput = cmake --build . --config Debug --target voice_encoder --dry-run 2>&1
            if ($LASTEXITCODE -eq 0) {
                Add-DiagnosticResult "Build" "Build Validation" "PASS" "Build commands valid"
            } else {
                Add-DiagnosticResult "Build" "Build Validation" "WARNING" "Build validation failed" "Check compiler and dependencies"
            }
        } else {
            Add-DiagnosticResult "Build" "CMake Configuration" "FAIL" "Configuration failed: $($cmakeOutput -join '; ')" "Check CMakeLists.txt and dependencies"
        }
    } catch {
        Add-DiagnosticResult "Build" "Build Test" "FAIL" "Exception: $_" "Check build environment"
    } finally {
        Set-Location ..
        if (Test-Path $tempBuildDir) {
            Remove-Item $tempBuildDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

function Export-DiagnosticReport {
    $reportPath = "diagnostic_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"
    
    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>ENSC 429 Diagnostic Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }
        .category { margin: 20px 0; }
        .test { margin: 5px 0; padding: 5px; border-left: 3px solid #ccc; }
        .pass { border-left-color: #4CAF50; background-color: #f1f8e9; }
        .fail { border-left-color: #f44336; background-color: #ffebee; }
        .warning { border-left-color: #ff9800; background-color: #fff3e0; }
        .info { border-left-color: #2196F3; background-color: #e3f2fd; }
        .details { font-size: 0.9em; color: #666; margin-top: 5px; }
        .recommendation { font-size: 0.9em; color: #d84315; margin-top: 5px; font-weight: bold; }
    </style>
</head>
<body>
    <div class="header">
        <h1>ENSC 429 Real-Time Vocoder - Diagnostic Report</h1>
        <p>Generated: $(Get-Date)</p>
        <p>System: $env:COMPUTERNAME ($env:USERNAME)</p>
    </div>
"@

    $currentCategory = ""
    foreach ($result in $DiagnosticResults) {
        if ($result.Category -ne $currentCategory) {
            if ($currentCategory -ne "") { $html += "</div>" }
            $html += "<div class='category'><h2>$($result.Category)</h2>"
            $currentCategory = $result.Category
        }
        
        $statusClass = $result.Status.ToLower()
        $html += "<div class='test $statusClass'>"
        $html += "<strong>$($result.Test)</strong>: $($result.Status)"
        if ($result.Details) {
            $html += "<div class='details'>$($result.Details)</div>"
        }
        if ($result.Recommendation) {
            $html += "<div class='recommendation'>Recommendation: $($result.Recommendation)</div>"
        }
        $html += "</div>"
    }
    
    $html += "</div></body></html>"
    
    $html | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Host "`nDiagnostic report exported to: $reportPath" -ForegroundColor $ColorInfo
}

function Show-Summary {
    Write-Host "`n=== DIAGNOSTIC SUMMARY ===" -ForegroundColor $ColorInfo
    
    $passed = ($DiagnosticResults | Where-Object { $_.Status -eq "PASS" }).Count
    $failed = ($DiagnosticResults | Where-Object { $_.Status -eq "FAIL" }).Count
    $warnings = ($DiagnosticResults | Where-Object { $_.Status -eq "WARNING" }).Count
    $total = $DiagnosticResults.Count
    
    Write-Host "Total Tests: $total" -ForegroundColor $ColorInfo
    Write-Host "Passed: $passed" -ForegroundColor $ColorSuccess
    Write-Host "Failed: $failed" -ForegroundColor $ColorError
    Write-Host "Warnings: $warnings" -ForegroundColor $ColorWarning
    
    if ($failed -eq 0 -and $warnings -eq 0) {
        Write-Host "`n🎉 All diagnostics passed! Your system is ready for development." -ForegroundColor $ColorSuccess
    } elseif ($failed -eq 0) {
        Write-Host "`n⚠️ All critical tests passed, but some warnings were found." -ForegroundColor $ColorWarning
        Write-Host "Your system should work, but consider addressing the warnings." -ForegroundColor $ColorWarning
    } else {
        Write-Host "`n❌ Some critical tests failed. Please address the failed items before proceeding." -ForegroundColor $ColorError
        Write-Host "Run the setup script to automatically install missing components." -ForegroundColor $ColorInfo
    }
}

# Main execution
Write-Host "ENSC 429 Real-Time Vocoder - System Diagnostics" -ForegroundColor $ColorInfo
Write-Host "=" * 50 -ForegroundColor $ColorInfo

Test-SystemEnvironment
Test-DevelopmentTools
Test-VisualStudio
Test-ProjectStructure
Test-AudioSystem
Test-BuildProcess

Show-Summary

if ($ExportReport) {
    Export-DiagnosticReport
}

Write-Host "`nDiagnostic completed. Run setup.ps1 to fix any issues automatically." -ForegroundColor $ColorInfo
