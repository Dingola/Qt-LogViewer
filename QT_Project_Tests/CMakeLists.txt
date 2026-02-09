cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

############################################
### Setup project                        ###
############################################

project(${MAIN_PROJECT_NAME}_Tests LANGUAGES CXX VERSION "0.0.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include CMake helper scripts
include(${CMAKE_SOURCE_DIR}/CMake/SourceGroups.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/BuildThirdPartyProject.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/IncludeThirdParty.cmake)

############################################
### Global Properties                    ###
############################################

# Disable CMake searching for modules
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Global properties for project organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include current directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)

############################################
### Documentation Configuration          ###
############################################

# Set the documentation sub-target name
set(DOC_OPTION_NAME ${MAIN_PROJECT_NAME}_Tests)
set(DOC_TARGET_NAME tests)

############################################
### Setup Project File Includes          ###
############################################

file(GLOB_RECURSE Headers
     "Headers/*.h"
)

file(GLOB_RECURSE Sources
     "main.cpp"
     "Sources/*.cpp"
)

include_directories(Headers Sources)

############################################
### Qt6 Configuration                    ###
############################################

find_package(Qt6 REQUIRED COMPONENTS Widgets Test LinguistTools)
qt_standard_project_setup()
#qt6_add_resources(RSCS resources.qrc)
#add_custom_target(gen_qrc DEPENDS ${RSCS})

############################################
### Clang-Format Configuration           ###
############################################

if(USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format" HINTS ${CLANG_TOOLS_PATH})
    if(CLANG_FORMAT)
        # Define a custom target for formatting code
        add_custom_target(${PROJECT_NAME}_run_clang_format_tests
            COMMAND ${CLANG_FORMAT}
            -style=file:${CMAKE_SOURCE_DIR}/Configs/.clang-format
            -i
            ${Headers}
            ${Sources}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting code with clang-format"
        )
        set_target_properties(${PROJECT_NAME}_run_clang_format_tests PROPERTIES FOLDER "_Tasks")
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
        add_custom_target(${PROJECT_NAME}_run_clang_tidy_tests
            COMMAND ${CLANG_TIDY}
			--config-file=${CMAKE_SOURCE_DIR}/Configs/.clang-tidy
            -p=${CMAKE_BINARY_DIR}
            ${Headers}
            ${Sources}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running clang-tidy for static analysis"
        )
        set_target_properties(${PROJECT_NAME}_run_clang_tidy_tests PROPERTIES FOLDER "_Tasks")
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
message(STATUS "  ${doc_sub_target_name}_BUILD_DOC:          ${${doc_sub_target_name}_BUILD_DOC}")
message(STATUS "  Qt6 Directory (Qt6_DIR):                  ${Qt6_DIR}")
message(STATUS "")
message(STATUS "-----------------------------------------------")
message(STATUS "")
message(STATUS "###############################################################")

############################################
### Setup executable build               ###
############################################

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Test)
include(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Doxygen.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GoogleTest.cmake)

if (WIN32)
    if (MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
		set_target_properties(${PROJECT_NAME} PROPERTIES 
			LINK_FLAGS "/SUBSYSTEM:CONSOLE"
			WIN32_EXECUTABLE ON
		)
    endif()
elseif (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        MACOSX_BUNDLE ON
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MAIN_PROJECT_NAME}_Tests)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_sources(${PROJECT_NAME}
    PRIVATE
		${Headers}
		${Sources}
)

if (WIN32)
    # Prefer Qt's deploy support if available
    if (DEFINED Qt6_DIR AND EXISTS "${Qt6_DIR}/Qt6DeploySupport.cmake")
        include("${Qt6_DIR}/Qt6DeploySupport.cmake")

        qt_generate_deploy_app_script(
            TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT deploy_script
        )

        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -DQT_DEPLOY_BIN_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>" -P "${deploy_script}"
            COMMENT "Deploying Qt runtime dependencies via Qt6DeploySupport..."
        )
    else()
        message(STATUS "Qt6DeploySupport.cmake not found. Falling back to windeployqt.")

        # Qt6_DIR points to <prefix>/lib/cmake/Qt6, go up three levels to get <prefix>
        get_filename_component(_qt_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
        find_program(WINDEPLOYQT_EXECUTABLE
            NAMES windeployqt windeployqt6
            HINTS "${_qt_prefix}/bin"
        )

        if (WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${PROJECT_NAME}>"
                COMMENT "Deploying Qt runtime dependencies via windeployqt..."
            )
        else()
            message(WARNING "windeployqt not found; no automatic deployment will be performed.")
        endif()
    endif()
endif()

############################################
### Setup source groups                  ###
############################################

GROUP_FILES("${Sources}" "Source Files")
GROUP_FILES("${Headers}" "Header Files")

# Specifies include libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${MAIN_PROJECT_NAME})

# Specifies include directories to use when compiling a given target
target_include_directories(${PROJECT_NAME} PUBLIC 
	${CMAKE_CURRENT_LIST_DIR} 
	${CMAKE_SOURCE_DIR}/QT_Project/Headers)

# Determine Qt installation prefix for locating translations
get_filename_component(_qt_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
if(NOT EXISTS "${_qt_prefix}")
    list(GET CMAKE_PREFIX_PATH 0 _first_prefix)
    if(_first_prefix)
        set(_qt_prefix "${_first_prefix}")
    endif()
endif()
set(QT_TRANSLATIONS_DIR "${_qt_prefix}/translations")

# Compute target runtime translations directory per platform using the TARGET name
if(WIN32)
    set(TEST_TRANSLATIONS_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations")
elseif(APPLE)
    set(TEST_TRANSLATIONS_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../MacOS/translations")
else()
    set(TEST_TRANSLATIONS_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations")
endif()

# Post-build: ensure dir exists and copy qt_en.qm
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${TEST_TRANSLATIONS_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${QT_TRANSLATIONS_DIR}/qt_en.qm"
            "${TEST_TRANSLATIONS_DIR}/qt_en.qm"
    COMMENT "Preparing test translations directory and copying qt_en.qm"
)
