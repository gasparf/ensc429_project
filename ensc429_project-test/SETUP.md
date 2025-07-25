# ENSC 429 Real-Time Vocoder - Setup Instructions

## Automated First-Time Setup

This project includes an automated setup system that will install all required dependencies on first build.

### Prerequisites

Before running the setup, ensure you have:

1. **Windows 10/11** (64-bit)
2. **Visual Studio 2019 or newer** with C++ development tools
   - OR **Visual Studio Build Tools** with MSVC compiler
3. **Git** (for cloning repositories)
4. **Administrator privileges** (for installing dependencies)

### Quick Start (Automated Setup)

1. **Clone the repository:**
   ```bash
   git clone <your-repo-url>
   cd ensc429_project-test
   ```

2. **Run the setup script:**
   ```bash
   # For PowerShell (Recommended)
   .\setup.ps1
   
   # For Command Prompt
   setup.bat
   ```

3. **Build the project:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

4. **Run the application:**
   ```bash
   .\Debug\voice_encoder.exe
   ```

### What the Setup Script Does

The automated setup will:

1. **Check System Requirements:**
   - Verify Windows version compatibility
   - Check for Visual Studio/Build Tools
   - Validate Git installation

2. **Install Build Tools (if missing):**
   - CMake (latest version)
   - Visual Studio Build Tools (if VS not found)
   - Windows SDK

3. **Install Audio Dependencies:**
   - PortAudio library (included as submodule)
   - Audio driver compatibility checks

4. **Setup Development Environment:**
   - Configure PATH variables
   - Set up include directories
   - Verify compilation toolchain

5. **Validate Installation:**
   - Test compile a simple program
   - Verify all dependencies are accessible
   - Generate build configuration

### Manual Setup (Alternative)

If you prefer manual installation or the automated script fails:

#### 1. Install CMake
```bash
# Using winget (Windows Package Manager)
winget install Kitware.CMake

# OR download from: https://cmake.org/download/
```

#### 2. Install Visual Studio Build Tools
```bash
# Using winget
winget install Microsoft.VisualStudio.2022.BuildTools

# OR download from: https://visualstudio.microsoft.com/downloads/
```

#### 3. Install Git (if not already installed)
```bash
winget install Git.Git
```

#### 4. Verify Installation
```bash
cmake --version
git --version
```

### Dependencies Included

- **PortAudio**: Audio I/O library (included as submodule)
- **MSVC Runtime**: C++ runtime libraries
- **Windows SDK**: Windows development headers

### Troubleshooting

#### Common Issues:

1. **"cmake not found"**
   - Restart terminal after installation
   - Check PATH environment variable

2. **"MSVC compiler not found"**
   - Install Visual Studio Build Tools
   - Restart system after installation

3. **"Audio devices not detected"**
   - Check audio driver installation
   - Run as administrator

4. **Build fails with linker errors**
   - Ensure all dependencies are installed
   - Try cleaning build directory and rebuilding

#### Getting Help:

1. Run the diagnostic script: `.\diagnose.ps1`
2. Check the build log in `build/setup.log`
3. Ensure all prerequisites are met

### Project Structure

```
ensc429_project-test/
├── src/                     # Source code
│   ├── audio/              # Audio processing
│   ├── dsp/                # Digital signal processing
│   └── main.cpp            # Main application
├── portaudio/              # PortAudio library (submodule)
├── scripts/                # Setup and utility scripts
├── build/                  # Build output (created during build)
├── setup.ps1              # Main setup script (PowerShell)
├── setup.bat               # Setup script (Batch)
└── CMakeLists.txt          # Build configuration
```

### Advanced Configuration

For developers wanting to customize the build:

1. **Custom PortAudio Build:**
   ```bash
   cmake .. -DPORTAUDIO_BUILD_SHARED=ON
   ```

2. **Release Build:**
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

3. **Custom Install Directory:**
   ```bash
   cmake .. -DCMAKE_INSTALL_PREFIX=C:\MyVocoder
   ```

### System Requirements

- **OS**: Windows 10 (1909+) or Windows 11
- **CPU**: x64 processor with SSE2 support
- **RAM**: 4GB minimum, 8GB recommended
- **Disk**: 2GB free space for dependencies
- **Audio**: DirectSound or WASAPI compatible audio device

### License and Credits

- PortAudio: MIT License
- Project Code: [Your License]
