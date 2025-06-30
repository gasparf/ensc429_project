# FindPortAudio.cmake - Locate PortAudio library
# This module defines
#  PORTAUDIO_LIBRARIES - PortAudio libraries
#  PORTAUDIO_INCLUDE_DIRS - PortAudio include directories
#  PortAudio::PortAudio - imported target (if found)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_PORTAUDIO QUIET portaudio-2.0)
endif()

find_path(PORTAUDIO_INCLUDE_DIR
    NAMES portaudio.h
    HINTS ${PC_PORTAUDIO_INCLUDEDIR} ${PC_PORTAUDIO_INCLUDE_DIRS}
)

find_library(PORTAUDIO_LIBRARY
    NAMES portaudio
    HINTS ${PC_PORTAUDIO_LIBDIR} ${PC_PORTAUDIO_LIBRARY_DIRS}
)

set(PORTAUDIO_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIR})
set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortAudio DEFAULT_MSG
    PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR)

mark_as_advanced(PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR)

if(PortAudio_FOUND AND NOT TARGET PortAudio::PortAudio)
    add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
    set_target_properties(PortAudio::PortAudio PROPERTIES
        IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}")
endif()

