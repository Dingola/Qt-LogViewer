# QtWidgetsCommonLib Integration
set(THIRD_PARTY_TARGET "QtWidgetsCommonLib")
set(GIT_TAG "main")
set(PROJECT_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")

# Paths
set(QtWidgetsCommonLib_INSTALL_ROOT "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install")
set(QtWidgetsCommonLib_INCLUDE_DIR "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/include")
set(QtWidgetsCommonLib_LIBRARY     "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/QtWidgetsCommonLib.lib")
set(QtWidgetsCommonLib_DIR "")

# Dependencies lookup helper
file(GLOB INSTALLED_THIRD_PARTY_PATHS
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
)
list(APPEND CMAKE_PREFIX_PATH ${INSTALLED_THIRD_PARTY_PATHS} "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

# Initialize status flags
set(SHOULD_ADD_SOURCE ON)
set(PACKAGE_FOUND_INSTALLED OFF)

# 1. Try to find installed package FIRST (unless we force source inclusion)
if(NOT ${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)
    find_package(QtWidgetsCommonLib HINTS ${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/QtWidgetsCommonLib NO_DEFAULT_PATHS)
    
    if(QtWidgetsCommonLib_FOUND)
        message(STATUS "[QtWidgetsCommonLib] Found installed package. Skipping download/source inclusion.")
        set(SHOULD_ADD_SOURCE OFF)
        set(PACKAGE_FOUND_INSTALLED ON)
    endif()
endif()

# 2. Download/Build (Executed if forced OR not found installed)
if(SHOULD_ADD_SOURCE)
    # Check if source is missing or we just want to ensure it's up to date/built
    if(NOT EXISTS "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_src/CMakeLists.txt")
        message(STATUS "[QtWidgetsCommonLib] Source not found (or forced refresh). Downloading...")
    else()
        message(STATUS "[QtWidgetsCommonLib] Checking/Updating library...")
    endif()

    set(CMAKE_ARGS "-D ${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=QtWidgetsCommonLib")

    build_third_party_project(
        false
        ${THIRD_PARTY_TARGET}
        https://github.com/Dingola/QtWidgetsCommonLib.git
        ${GIT_TAG}
        ${THIRD_PARTY_TARGET_DIR}
        ${CMAKE_BUILD_TYPE}
        ${CMAKE_ARGS}
    )
endif()

# 3. Integration Step
if(${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)
    # A) Add Source Code directly to Solution
    if(EXISTS "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_src/CMakeLists.txt")
        message(STATUS "[QtWidgetsCommonLib] Adding source to solution (Enabled via ${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)...")

        set(BACKUP_MAIN_PROJECT_NAME ${MAIN_PROJECT_NAME})
        set(MAIN_PROJECT_NAME ${THIRD_PARTY_TARGET})
        
        # Force static library build for the sub-project
        set(${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE "static_library" CACHE STRING "Force static library" FORCE)

        add_subdirectory(
            "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_src" 
            "${CMAKE_BINARY_DIR}/ThirdParty/${THIRD_PARTY_TARGET}_Solution" 
            EXCLUDE_FROM_ALL
        )
        
        set(MAIN_PROJECT_NAME ${BACKUP_MAIN_PROJECT_NAME})
    else()
        message(FATAL_ERROR "[QtWidgetsCommonLib] Source code missing even after download attempt.")
    endif()

else()
    # B) Use Binary (find_package)
    # If we didn't find it in step 1, we try again now (after build_third_party_project has run)
    if(NOT PACKAGE_FOUND_INSTALLED)
        
        file(GLOB NEWLY_INSTALLED_PATHS
            "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
            "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
        )
        list(APPEND CMAKE_PREFIX_PATH ${NEWLY_INSTALLED_PATHS})

        find_package(QtWidgetsCommonLib HINTS ${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/QtWidgetsCommonLib NO_DEFAULT_PATHS)
        
        if(QtWidgetsCommonLib_FOUND)
             message(STATUS "[QtWidgetsCommonLib] Found package after build.")
        else()
             message(FATAL_ERROR "[QtWidgetsCommonLib] Failed to find package even after build.")
        endif()
    endif()
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC QtWidgetsCommonLib)
