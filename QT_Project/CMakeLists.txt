cmake_minimum_required(VERSION 4.1.1 FATAL_ERROR)

############################################
### Project Configuration                ###
############################################

project(${MAIN_PROJECT_NAME} LANGUAGES CXX VERSION "0.0.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(Config.h.in Config.h)

# Include CMake helper scripts
include(${CMAKE_SOURCE_DIR}/CMake/SourceGroups.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/BuildThirdPartyProject.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/SanitizerConfig.cmake)

############################################
### Global Properties                    ###
############################################

# Disable CMake searching for modules
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Global properties for project organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include current directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Export all symbols for Windows
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

############################################
### Documentation Configuration          ###
############################################

# Set the documentation sub-target name
set(DOC_OPTION_NAME ${MAIN_PROJECT_NAME})
set(DOC_TARGET_NAME project)

############################################
### Setup Project File Includes          ###
############################################

file(GLOB_RECURSE Headers
     "Headers/*.h"
     "Headers/*/*.h"
)

file(GLOB_RECURSE Sources
     "main.cpp"
     "Sources/*.cpp"
     "Headers/*/*.cpp"
)

file(GLOB_RECURSE Forms
    "Forms/*.ui"
    "Forms/*/*.ui"
)

include_directories(Headers Sources)

############################################
### Qt6 Configuration                    ###
############################################

if ("$ENV{QT6_DIR}" STREQUAL "")
	set(QT6_DIR "E:\\Qt\\6.8.0\\msvc2022_64\\")
else()
	set(QT6_DIR "$ENV{QT6_DIR}")
endif()

if (EXISTS ${QT6_DIR})
	set(CMAKE_PREFIX_PATH ${QT6_DIR})
else()
	message(WARNING "The specified qt6 path '${QT6_DIR}' does not exist")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Svg)
qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)
qt6_add_resources(RSCS resources.qrc)
add_custom_target(gen_qrc DEPENDS ${RSCS})

qt6_wrap_ui(UI_HEADERS ${Forms})

############################################
### Translation Configuration            ###
############################################

set(TS_DIR QT_Project/Resources/Translations)
set(TS_FILES ${TS_DIR}/app_de.ts ${TS_DIR}/app_en.ts)

# Ensure the translations directory exists
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${TS_DIR})

# Define a custom target for updating translations
add_custom_target(_translations_update
    COMMAND ${QT6_DIR}/bin/lupdate ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Updating translation files"
)

# Define a custom target for compiling translations
add_custom_target(_translations_compile
    DEPENDS ${TS_FILES}
    COMMAND ${QT6_DIR}/bin/lrelease ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compiling translation files"
)

# Ensure the compiled .qm files are part of the build
set(QM_FILES "")
foreach(TS_FILE ${TS_FILES})
    get_filename_component(QM_FILE ${TS_FILE} NAME_WE)
    set(QM_FILES ${QM_FILES} ${CMAKE_SOURCE_DIR}/${TS_DIR}/${QM_FILE}.qm)
endforeach()

# Ensure the target directory exists before copying
add_custom_command(TARGET _translations_compile POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/translations
    COMMAND ${CMAKE_COMMAND} -E copy ${QM_FILES} $<TARGET_FILE_DIR:${PROJECT_NAME}>/translations
)

############################################
### Clang-Format Configuration           ###
############################################

if(USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format" HINTS ${CLANG_TOOLS_PATH})
    if(CLANG_FORMAT)
        # Define a custom target for formatting code
        add_custom_target(_run_clang_format_project
            COMMAND ${CLANG_FORMAT}
            -style=file:${CMAKE_SOURCE_DIR}/Configs/.clang-format
            -i
            ${Headers}
            ${Sources}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting code with clang-format"
        )
    else()
        message(WARNING "clang-format not found. Please ensure clang-format is installed and the path is set correctly.")
    endif()
endif()

############################################
### Clang-Tidy Configuration             ###
############################################

if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY "clang-tidy" HINTS ${CLANG_TOOLS_PATH})
    if(CLANG_TIDY)
		# Define a custom target for running clang-tidy
		add_custom_target(_run_clang_tidy_project
			COMMAND ${CLANG_TIDY}
			--config-file=${CMAKE_SOURCE_DIR}/Configs/.clang-tidy
			-p=${CMAKE_BINARY_DIR}
			${Headers}
			${Sources}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "Running clang-tidy for static analysis"
		)
    else()
        message(WARNING "clang-tidy not found. Please ensure clang-tidy is installed and the path is set correctly.")
    endif()
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
message(STATUS "  ${doc_sub_target_name}_BUILD_DOC:          ${${doc_sub_target_name}_BUILD_DOC}")
message(STATUS "  Qt6 Directory (QT6_DIR env):              ${QT6_DIR}")
message(STATUS "  Translation Files Directory:              ${CMAKE_SOURCE_DIR}/${TS_DIR}")
message(STATUS "  Translation Files:                        ${TS_FILES}")
message(STATUS "")
message(STATUS "-----------------------------------------------")
message(STATUS "")
message(STATUS "###############################################################")

############################################
### Setup Target Build Type              ###
############################################

if (${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE STREQUAL executable)
	add_executable(${PROJECT_NAME})
elseif (${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE STREQUAL dynamic_library)
	add_library(${PROJECT_NAME} SHARED)
elseif (${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE STREQUAL static_library)
	add_library(${PROJECT_NAME})
else()
	message(FATAL_ERROR "Build type not specified")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Svg)
include(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Doxygen.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SimpleQtLogger.cmake)

if (WIN32)
    if (MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set_target_properties(${PROJECT_NAME} PROPERTIES 
                LINK_FLAGS "/SUBSYSTEM:CONSOLE"
            )
        endif()
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        WIN32_EXECUTABLE ON
    )
elseif (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        MACOSX_BUNDLE ON
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MAIN_PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${Headers}
        ${Sources}
        ${RSCS}
        ${UI_HEADERS}
)

if (WIN32)
	# windeployqt takes an .exe file or a directory that contains an .exe file 
	# as an argument, and scans the executable for dependencies
	#
	# Link: https://doc.qt.io/qt-6/windows-deployment.html
	#
	if (${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE STREQUAL executable)
		# Retrieve the absolute path to qmake and then use that path to find
		# the windeployqt executable
        find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT6_DIR}/bin")
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT6_DIR}/bin")

		# Run windeployqt immediately after build
		add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:${PROJECT_NAME}>\"
			COMMENT "Running windeployqt..."
		)
		
		# Run windeployqt immediately after build
		#add_custom_command(TARGET ${PROJECT_NAME}
		#	POST_BUILD
		#	COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:${PROJECT_NAME}>\"
		#	COMMENT "Running windeployqt..."
		#)
	endif()
endif()

############################################
### Setup source groups                  ###
############################################

group_files("${Sources}" "Source Files")
group_files("${Headers}" "Header Files")
group_files("${Forms}" "UI Files")
group_files("${UI_HEADERS}" "Auto-Generated UI Headers")

# Specifies include directories to use when compiling a given target
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Headers>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Headers/Private>
)

############################################
### Install rules                        ###
############################################

include(CMake/install.cmake)
