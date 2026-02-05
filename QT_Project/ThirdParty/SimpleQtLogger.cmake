# SimpleQtLogger Integration
set(THIRD_PARTY_TARGET "SimpleQtLogger")
set(GIT_TAG "main")
set(PROJECT_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")
set(CMAKE_ARGS "-D ${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=SimpleQtLogger")
set(SimpleQtLogger_INSTALL_ROOT "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install")
set(SimpleQtLogger_INCLUDE_DIR "${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/include")
set(SimpleQtLogger_LIBRARY     "${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/SimpleQtLogger.lib")
set(SimpleQtLogger_DIR "")

# Dynamically find all installed third-party packages to help find_dependency()
# matching the pattern: ThirdParty/<Project>_<Tag>/<Target>_install/<Config>
file(GLOB _installed_third_parties
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
)
list(APPEND CMAKE_PREFIX_PATH ${_installed_third_parties} "${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

find_package(SimpleQtLogger HINTS ${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/SimpleQtLogger NO_DEFAULT_PATHS)

if(SimpleQtLogger_FOUND)
    message("SimpleQtLogger found")
else()
    message("SimpleQtLogger not found. Downloading and invoking cmake ..")
    build_third_party_project(
        false
        ${THIRD_PARTY_TARGET}
        https://github.com/Dingola/SimpleQtLogger.git
        ${GIT_TAG}
        ${THIRD_PARTY_TARGET_DIR}
        ${CMAKE_BUILD_TYPE}
        ${CMAKE_ARGS}
    )

    # Re-scan third-party installs after the build to pick up dependencies (SimpleCppLogger) that were just installed
    file(GLOB _installed_third_parties
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
    )
    list(APPEND CMAKE_PREFIX_PATH ${_installed_third_parties} "${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

    find_package(SimpleQtLogger REQUIRED HINTS ${SimpleQtLogger_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/SimpleQtLogger NO_DEFAULT_PATHS)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC SimpleQtLogger)
