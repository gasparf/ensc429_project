cmake_minimum_required(VERSION 3.10...3.28)

project(voice_encoder)

# Project information
set(PROJECT_VERSION "1.0.0")
set(PROJECT_DESCRIPTION "ENSC 429 Real-Time Vocoder with DSP Effects")

# Display project information
message(STATUS "=== ENSC 429 Real-Time Vocoder ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Description: ${PROJECT_DESCRIPTION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "===================================")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
    message(STATUS "Setting build type to Debug (default)")
endif()

# Platform-specific configurations
if(WIN32)
    message(STATUS "Configuring for Windows...")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
    # Windows-specific compiler flags
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
        message(STATUS "Using MSVC compiler with optimizations")
    endif()
elseif(APPLE)
    message(STATUS "Configuring for macOS...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif(UNIX)
    message(STATUS "Configuring for Linux...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# === DEPENDENCY MANAGEMENT ===
message(STATUS "Checking dependencies...")

# Function to check if a command exists
function(check_command_exists COMMAND_NAME RESULT_VAR)
    execute_process(
        COMMAND ${COMMAND_NAME} --version
        RESULT_VARIABLE CMD_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(CMD_RESULT EQUAL 0)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Check for Git (required for submodules)
check_command_exists("git" GIT_FOUND)
if(NOT GIT_FOUND)
    message(WARNING "Git not found! Submodule initialization may fail.")
    message(STATUS "Please install Git from: https://git-scm.com/download/win")
endif()

# Auto-initialize PortAudio submodule if needed
set(PORTAUDIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/portaudio")
if(NOT EXISTS "${PORTAUDIO_DIR}/include/portaudio.h")
    message(STATUS "PortAudio not found, attempting to initialize submodule...")
    
    if(GIT_FOUND)
        # Check if we're in a git repository
        execute_process(
            COMMAND git rev-parse --git-dir
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE IS_GIT_REPO
            OUTPUT_QUIET
            ERROR_QUIET
        )
        
        if(IS_GIT_REPO EQUAL 0)
            message(STATUS "Initializing PortAudio submodule...")
            execute_process(
                COMMAND git submodule update --init --recursive portaudio
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE SUBMODULE_RESULT
            )
            
            if(SUBMODULE_RESULT EQUAL 0)
                message(STATUS "✓ PortAudio submodule initialized successfully")
            else()
                message(FATAL_ERROR "Failed to initialize PortAudio submodule. Please run: git submodule update --init --recursive")
            endif()
        else()
            message(FATAL_ERROR "Not in a git repository and PortAudio not found. Please ensure PortAudio is available or run from a git repository.")
        endif()
    else()
        message(FATAL_ERROR "Git not available and PortAudio not found. Please install Git and retry, or manually provide PortAudio.")
    endif()
else()
    message(STATUS "✓ PortAudio found")
endif()

# Verify PortAudio is properly available
if(NOT EXISTS "${PORTAUDIO_DIR}/include/portaudio.h")
    message(FATAL_ERROR "PortAudio header not found at ${PORTAUDIO_DIR}/include/portaudio.h")
endif()

# Check for required system libraries on Windows
if(WIN32)
    # Check for Windows SDK
    if(MSVC)
        message(STATUS "Checking Windows SDK...")
        # This will fail during configuration if Windows SDK is not available
        try_compile(WINDOWS_SDK_AVAILABLE
            ${CMAKE_BINARY_DIR}/test_sdk
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test_windows_sdk.cpp
        )
        
        if(WINDOWS_SDK_AVAILABLE)
            message(STATUS "✓ Windows SDK available")
        else()
            message(WARNING "Windows SDK may not be properly installed")
            message(STATUS "Please install Windows 10/11 SDK from Visual Studio Installer")
        endif()
    endif()
endif()

# === SOURCE FILE DISCOVERY ===
message(STATUS "Discovering source files...")

# Find all source files recursively
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*.hpp"
    "src/*.h"
)

# Organize sources by directory for better build output
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SOURCES})

# Display found sources
list(LENGTH SOURCES SOURCE_COUNT)
message(STATUS "Found ${SOURCE_COUNT} source files:")
foreach(SOURCE_FILE ${SOURCES})
    file(RELATIVE_PATH REL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_FILE})
    message(STATUS "  - ${REL_SOURCE}")
endforeach()

# === EXECUTABLE CREATION ===
message(STATUS "Creating executable target...")

# Create the main executable
add_executable(${PROJECT_NAME} ${SOURCES} "src/dsp/BANDPASS_coeff.cpp")

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "voice_encoder"
    DEBUG_POSTFIX "_debug"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# === PORTAUDIO CONFIGURATION ===
message(STATUS "Configuring PortAudio...")

# PortAudio build options
set(PA_BUILD_SHARED OFF CACHE BOOL "Build shared library")
set(PA_BUILD_STATIC ON CACHE BOOL "Build static library")
set(PA_BUILD_TESTS OFF CACHE BOOL "Build test programs")
set(PA_BUILD_EXAMPLES OFF CACHE BOOL "Build example programs")

# Windows-specific PortAudio fixes
if(WIN32)
    # Use only the most compatible Windows audio backend (MME)
    # Disable all modern backends that have Windows SDK compatibility issues
    set(PA_USE_DS OFF CACHE BOOL "Enable support for DirectSound")
    set(PA_USE_WASAPI OFF CACHE BOOL "Enable support for WASAPI") 
    set(PA_USE_WDMKS OFF CACHE BOOL "Enable support for WDM/KS")
    set(PA_USE_WMME ON CACHE BOOL "Enable support for MME")
    
    # Add preprocessor definitions to fix Windows SDK issues
    # Note: Removed PA_NO_* definitions as they're deprecated in newer PortAudio
    add_definitions(-DPAWIN_USE_WDMKS_DEVICE_INFO=0)
    add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
    
    message(STATUS "✓ PortAudio configured for Windows with MME support only")
    message(STATUS "  - DirectSound: DISABLED (PA_USE_DS=OFF)")
    message(STATUS "  - WASAPI: DISABLED (PA_USE_WASAPI=OFF)")
    message(STATUS "  - WDM/KS: DISABLED (PA_USE_WDMKS=OFF)")
    message(STATUS "  - MME: ENABLED (PA_USE_WMME=ON)")
endif()

# Disable PortAudio warnings in MSVC
if(MSVC)
    set(PA_DISABLE_WARNINGS ON CACHE BOOL "Disable PortAudio warnings")
    
    # Add specific compiler flags to suppress problematic PortAudio code
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# Add PortAudio subdirectory
add_subdirectory(${PORTAUDIO_DIR} EXCLUDE_FROM_ALL)

# === LINKING AND INCLUDES ===
message(STATUS "Configuring linking and include directories...")

# Link PortAudio
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio_static)

# Platform-specific system libraries
if(WIN32)
    # Link only compatible Windows audio libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm      # Windows Multimedia API (MME)
        ole32      # OLE32 for COM interfaces
        oleaut32   # OLE Automation
        advapi32   # Advanced Windows API
        shell32    # Shell API
        user32     # User interface API
        # Note: Removed dsound (DirectSound) due to Windows SDK compatibility issues
    )
    message(STATUS "✓ Windows system libraries linked (MME, OLE32, etc.)")
    message(STATUS "  Note: DirectSound libraries excluded due to compatibility issues")
elseif(APPLE)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COREAUDIO_LIBRARY}
        ${AUDIOTOOLBOX_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
    message(STATUS "✓ macOS system libraries linked")
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ALSA_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${ALSA_INCLUDE_DIRS})
    message(STATUS "✓ Linux ALSA libraries linked")
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtos
    ${PORTAUDIO_DIR}/include
)

# === COMPILER-SPECIFIC OPTIMIZATIONS ===
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Applying release optimizations...")
    
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /O2          # Optimize for speed
            /Ob2         # Inline any suitable function
            /Ot          # Favor fast code
            /Oy          # Omit frame pointers
            /GL          # Whole program optimization
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            /LTCG        # Link-time code generation
        )
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O3          # Maximum optimization
            -DNDEBUG     # Disable debug assertions
            -march=native # Optimize for current CPU
        )
    endif()
endif()

# === INSTALLATION RULES ===
# Install the executable
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install documentation
install(FILES README.md SETUP.md
    DESTINATION doc
    OPTIONAL
)

# === BUILD VALIDATION ===
message(STATUS "Performing build validation...")

# Create test file for compilation validation
file(WRITE ${CMAKE_BINARY_DIR}/test_compile.cpp
"#include <iostream>
#include <vector>
#include <cmath>
int main() { 
    std::cout << \"Build test successful\" << std::endl; 
    return 0; 
}")

try_compile(COMPILE_TEST_RESULT
    ${CMAKE_BINARY_DIR}/test_compile
    ${CMAKE_BINARY_DIR}/test_compile.cpp
)

if(COMPILE_TEST_RESULT)
    message(STATUS "✓ Compilation test passed")
else()
    message(WARNING "Compilation test failed - there may be issues with the build environment")
endif()

# === SUMMARY ===
message(STATUS "")
message(STATUS "=== BUILD CONFIGURATION SUMMARY ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Source Files: ${SOURCE_COUNT}")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/bin/voice_encoder${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "")
message(STATUS "PortAudio: Static library linked")
if(WIN32)
    message(STATUS "Platform: Windows (${CMAKE_SYSTEM_VERSION})")
    message(STATUS "Audio Backend: MME (Windows Multimedia Extensions)")
    message(STATUS "System Libraries: winmm, ole32, oleaut32, advapi32, shell32, user32")
elseif(APPLE)
    message(STATUS "Platform: macOS")
    message(STATUS "System Libraries: CoreAudio, AudioToolbox, AudioUnit, CoreFoundation")
else()
    message(STATUS "Platform: Linux")
    message(STATUS "System Libraries: ALSA")
endif()
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  cmake --build . --config ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "To run:")
if(WIN32)
    message(STATUS "  .\\bin\\voice_encoder.exe")
else()
    message(STATUS "  ./bin/voice_encoder")
endif()
message(STATUS "=====================================")