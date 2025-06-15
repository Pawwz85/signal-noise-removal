cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0.22621.0 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


project(metody_dekompozycji_sygnalu CXX)


################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Common utils
################################################################################
#include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
#include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################

set(PROJECT_NAME metody_dekompozycji_sygnalu)

################################################################################
# Source groups
################################################################################
set(headers
    "App.h"
    "csv_parser.h"
    "scenes.h"
    "signals.h"
    "Style.h"
)
source_group("Pliki nagłówkowe" FILES ${headers})

set(sources
    "main.cpp"
    "scenes.cpp"
)
source_group("Pliki źródłowe" FILES ${sources})

set(ALL_FILES
    ${headers}
    ${sources}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

#use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE metodydekompozycjisygnału)

###
# Link FLTK
###
find_package(FLTK REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fltk
)

