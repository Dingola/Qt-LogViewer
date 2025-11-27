cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

############################################
### Project Configuration                ###
############################################

set(PROJECT_NAME "")
set(PROJECT_VER "0.0.0")
set(PROJECT_VER_MAJOR "")
set(PROJECT_VER_MINOR "")
set(PROJECT_VER_PATCH "")

# Set PROJECT_NAME to the folder name if it is not provided externally or is empty
if (NOT PROJECT_NAME)
    get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
endif()
set(MAIN_PROJECT_NAME "${PROJECT_NAME}" CACHE STRING "Main project name")

project(${MAIN_PROJECT_NAME}_Solution LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Suppress specific compiler warnings
#add_definitions("/wd4114")

# Include CMake helper scripts
include(${CMAKE_SOURCE_DIR}/CMake/BuildThirdPartyProject.cmake)

############################################
### Build Options                        ###
############################################

# Set the build type for the target project
set(${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE "executable" CACHE STRING "Build target options")
set_property(CACHE ${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE PROPERTY STRINGS executable dynamic_library static_library)

# Option to build the test project
option(${MAIN_PROJECT_NAME}_BUILD_TEST_PROJECT "Build test project" OFF)

# Option to use clang-format
option(USE_CLANG_FORMAT "Use clang-format for code formatting" OFF)

# Option to use clang-tidy
option(USE_CLANG_TIDY "Use clang-tidy for static analysis" OFF)

# Option to specify the path to clang tools
set(CLANG_TOOLS_PATH "" CACHE PATH "Path to clang tools (clang-format, clang-tidy)")

# Option to use ccache
option(USE_CCACHE "Use ccache for faster recompilation" OFF)

# Path to ThirdParty directories
if (WIN32)
    set(DEFAULT_THIRD_PARTY_PATH "$ENV{USERPROFILE}/ThirdParty")
else()
    set(DEFAULT_THIRD_PARTY_PATH "$ENV{HOME}/ThirdParty")
endif()
set(THIRD_PARTY_INCLUDE_DIR ${DEFAULT_THIRD_PARTY_PATH} CACHE PATH "Third Party Directory")

if (USE_CCACHE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CCache.cmake)
endif()

############################################
### Global Properties                    ###
############################################

# Global properties for project organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include current directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)

############################################
### Build Type Configuration             ###
############################################

# Set the build type if not set
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")
endif()

############################################
### Configuration Information            ###
############################################

message(STATUS "###############################################################")
message(STATUS "###          Configuration Information")
message(STATUS "###          Project: ${PROJECT_NAME}")
message(STATUS "###############################################################")
message(STATUS "")
message(STATUS "  CMake Version:                ${CMAKE_VERSION}")
message(STATUS "  CMake Prefix Path:            ${CMAKE_PREFIX_PATH}")
message(STATUS "  CMake Install Prefix Path:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Host System Name:             ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "  Host System Version:          ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "  Target System Name:           ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Target System Version:        ${CMAKE_SYSTEM_VERSION}")
message(STATUS "  Source Directory:             ${CMAKE_SOURCE_DIR}")
message(STATUS "  Build Type:                   ${CMAKE_BUILD_TYPE}")
message(STATUS "  Toolchain File:               ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  C++ Compiler:                 ${CMAKE_CXX_COMPILER}")
message(STATUS "  C Compiler:                   ${CMAKE_C_COMPILER}")
message(STATUS "  Build Tool:                   ${CMAKE_BUILD_TOOL}")
message(STATUS "  Module Path:                  ${CMAKE_MODULE_PATH}")
message(STATUS "  Binary Directory:             ${CMAKE_BINARY_DIR}")
message(STATUS "  Current Source Directory:     ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Current Binary Directory:     ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "")
message(STATUS "-----------------------------------------------")
message(STATUS "")
message(STATUS "  Third Party Include Directory:            ${THIRD_PARTY_INCLUDE_DIR}")
message(STATUS "  ${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE:  ${${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE}")
message(STATUS "  ${MAIN_PROJECT_NAME}_BUILD_TEST_PROJECT: ${${MAIN_PROJECT_NAME}_BUILD_TEST_PROJECT}")
message(STATUS "")
message(STATUS "-----------------------------------------------")
message(STATUS "")
message(STATUS "###############################################################")

############################################
### Thread Configuration for Linux       ###
############################################

if (UNIX)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

############################################
### Subdirectories                       ###
############################################

# Add the main project
add_subdirectory(QT_Project)

# Add the test project conditionally
if (${MAIN_PROJECT_NAME}_BUILD_TEST_PROJECT)
  add_subdirectory(QT_Project_Tests)
  set(startup_project ${MAIN_PROJECT_NAME}_Tests)
else()
  set(startup_project ${MAIN_PROJECT_NAME})
endif()

# Set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${startup_project})

message(STATUS "Startup project: ${startup_project}")
