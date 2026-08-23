cmake_minimum_required(VERSION 3.22)
include(ExternalProject)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Set the project name
set(CMAKE_PROJECT_NAME U585)

# Core project settings
project(${CMAKE_PROJECT_NAME})

# Forward article demo selection into the NonSecure ExternalProject.
set(U585_ACTIVE_DEMO "3" CACHE STRING "Active U585 article demo (3-31)")
set(U585_EXP04_ENABLE_FAULT_TRIGGER OFF CACHE BOOL "Enable deliberate fault trigger in demo 4")

# Include external project file
include("mx-generated.cmake")

