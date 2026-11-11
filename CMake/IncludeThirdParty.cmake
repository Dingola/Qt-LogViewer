# Macro to handle third-party library integration (Download, Build, Include)
# Usage:
#   include_third_party_library(
#       TARGET_NAME       <Name of the library, e.g. SimpleQtLogger>
#       GIT_URL           <Git URL to clone>
#       GIT_TAG           <Git Tag/Branch>
#       LINK_TYPE         <PUBLIC/PRIVATE/INTERFACE>
#   )
# Optional local development override:
#   -D<TARGET_NAME>_SOURCE_DIR=<path-to-local-source>
#   -D<TARGET_NAME>_USE_LOCAL_SOURCE=<ON/OFF>
macro(include_third_party_library TARGET_NAME GIT_URL GIT_TAG LINK_TYPE)

    # An explicitly supplied local source directory has highest priority. This supports
    # cross-repository development without requiring an intermediate commit, push, or install.
    set(
        ${TARGET_NAME}_SOURCE_DIR
        ""
        CACHE PATH
        "Optional local source directory for ${TARGET_NAME}"
    )
    set(
        ${TARGET_NAME}_USE_LOCAL_SOURCE
        ON
        CACHE BOOL
        "Use ${TARGET_NAME}_SOURCE_DIR when it is configured"
    )
    set(_LOCAL_SOURCE_DIR "${${TARGET_NAME}_SOURCE_DIR}")

    if(${TARGET_NAME}_USE_LOCAL_SOURCE AND NOT _LOCAL_SOURCE_DIR STREQUAL "")
        get_filename_component(
            _LOCAL_SOURCE_DIR
            "${_LOCAL_SOURCE_DIR}"
            ABSOLUTE
            BASE_DIR "${CMAKE_SOURCE_DIR}"
        )

        if(NOT EXISTS "${_LOCAL_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR
                "[${TARGET_NAME}] Local source directory does not contain CMakeLists.txt: "
                "${_LOCAL_SOURCE_DIR}"
            )
        endif()

        if(TARGET ${TARGET_NAME})
            message(STATUS
                "[${TARGET_NAME}] Target already exists; reusing it for the local source override."
            )
        else()
            message(STATUS
                "[${TARGET_NAME}] Using local source directory: ${_LOCAL_SOURCE_DIR}"
            )

            set(BACKUP_MAIN_PROJECT_NAME ${MAIN_PROJECT_NAME})
            set(MAIN_PROJECT_NAME ${TARGET_NAME})
            set(${TARGET_NAME}_BUILD_TARGET_TYPE "static_library" CACHE STRING
                "Force library target type for local dependency development" FORCE)

            add_subdirectory(
                "${_LOCAL_SOURCE_DIR}"
                "${CMAKE_BINARY_DIR}/ThirdParty/${TARGET_NAME}_Local"
                EXCLUDE_FROM_ALL
            )

            set(MAIN_PROJECT_NAME ${BACKUP_MAIN_PROJECT_NAME})
        endif()

        if(NOT TARGET ${TARGET_NAME})
            message(FATAL_ERROR
                "[${TARGET_NAME}] LOCAL_SOURCE_TARGET_MISSING: local source directory did not "
                "define the expected target '${TARGET_NAME}': ${_LOCAL_SOURCE_DIR}"
            )
        endif()
    else()

    # 1. Setup Paths & Variables
    set(PROJECT_DIR_NAME "${TARGET_NAME}_${GIT_TAG}")
    set(TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")
    set(INSTALL_ROOT "${TARGET_DIR}/${TARGET_NAME}_install")
    
    # Define standard variables for the library paths
    set(${TARGET_NAME}_INSTALL_ROOT "${INSTALL_ROOT}")
    set(${TARGET_NAME}_INCLUDE_DIR  "${INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/include")
    set(${TARGET_NAME}_LIBRARY      "${INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/${TARGET_NAME}.lib")
    set(${TARGET_NAME}_DIR          "") 

    # 2. Add search paths for this library and any other already installed libs
    file(GLOB ALL_INSTALLED_PATHS
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
    )
    list(APPEND CMAKE_PREFIX_PATH ${ALL_INSTALLED_PATHS} "${INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

    # 3. Status Flags
    set(SHOULD_ADD_SOURCE ON)
    set(PACKAGE_FOUND_INSTALLED OFF)

    # 4. Try to find installed package FIRST (unless Source is forced)
    if(NOT ${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)
        find_package(${TARGET_NAME} HINTS ${INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/${TARGET_NAME} NO_DEFAULT_PATHS)
        
        # Access the result variable dynamically
        if(${TARGET_NAME}_FOUND)
            message(STATUS "[${TARGET_NAME}] Found installed package. Skipping download/source inclusion.")
            set(SHOULD_ADD_SOURCE OFF)
            set(PACKAGE_FOUND_INSTALLED ON)
        endif()
    endif()

    # 5. Download/Build (Executed if forced OR not found installed)
    if(SHOULD_ADD_SOURCE)
        if(NOT EXISTS "${TARGET_DIR}/${TARGET_NAME}_src/CMakeLists.txt")
            message(STATUS "[${TARGET_NAME}] Source not found (or forced refresh). Downloading...")
        else()
            message(STATUS "[${TARGET_NAME}] Checking/Updating library...")
        endif()

        set(CMAKE_ARGS "-D ${TARGET_NAME}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=${TARGET_NAME}")

        # Assuming build_third_party_project is already included or available
        build_third_party_project(
            false
            ${TARGET_NAME}
            ${GIT_URL}
            ${GIT_TAG}
            ${TARGET_DIR}
            ${CMAKE_BUILD_TYPE}
            ${CMAKE_ARGS}
        )
    endif()

    # 6. Integration Step
    if(${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)
        # A) Add Source Code directly to Solution
        if(EXISTS "${TARGET_DIR}/${TARGET_NAME}_src/CMakeLists.txt")
            message(STATUS "[${TARGET_NAME}] Adding source to solution (Enabled via ${MAIN_PROJECT_NAME}_INCLUDE_THIRD_LIBS_INTO_SOLUTION)...")

            set(BACKUP_MAIN_PROJECT_NAME ${MAIN_PROJECT_NAME})
            set(MAIN_PROJECT_NAME ${TARGET_NAME})
            
            # Force static library build for the sub-project
            set(${TARGET_NAME}_BUILD_TARGET_TYPE "static_library" CACHE STRING "Force static library" FORCE)

            add_subdirectory(
                "${TARGET_DIR}/${TARGET_NAME}_src" 
                "${CMAKE_BINARY_DIR}/ThirdParty/${TARGET_NAME}_Solution" 
                EXCLUDE_FROM_ALL
            )
            
            set(MAIN_PROJECT_NAME ${BACKUP_MAIN_PROJECT_NAME})
        else()
            message(FATAL_ERROR "[${TARGET_NAME}] Source code missing even after download attempt.")
        endif()

    else()
        # B) Use Binary (find_package)
        if(NOT PACKAGE_FOUND_INSTALLED)
            
            # Refresh search paths (Important for transitive dependencies!)
            file(GLOB NEWLY_INSTALLED_PATHS
                "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
                "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
            )
            list(APPEND CMAKE_PREFIX_PATH ${NEWLY_INSTALLED_PATHS})

            find_package(${TARGET_NAME} HINTS ${INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/${TARGET_NAME} NO_DEFAULT_PATHS)
            
            if(${TARGET_NAME}_FOUND)
                 message(STATUS "[${TARGET_NAME}] Found package after build.")
            else()
                 message(FATAL_ERROR "[${TARGET_NAME}] Failed to find package even after build.")
            endif()
        endif()
    endif()

    endif()

    # 7. Link Library
    target_link_libraries(${PROJECT_NAME} ${LINK_TYPE} ${TARGET_NAME})

    unset(_LOCAL_SOURCE_DIR)

endmacro()
