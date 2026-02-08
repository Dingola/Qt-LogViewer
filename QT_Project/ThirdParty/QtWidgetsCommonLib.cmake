# QtWidgetsCommonLib Integration
set(THIRD_PARTY_TARGET "QtWidgetsCommonLib")
set(GIT_TAG "main")
set(PROJECT_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")
set(CMAKE_ARGS "-D ${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=QtWidgetsCommonLib")
set(QtWidgetsCommonLib_INSTALL_ROOT "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install")
set(QtWidgetsCommonLib_INCLUDE_DIR "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/include")
set(QtWidgetsCommonLib_LIBRARY     "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/QtWidgetsCommonLib.lib")
set(QtWidgetsCommonLib_DIR "")

# Dynamically find all installed third-party packages to help find_dependency()
# matching the pattern: ThirdParty/<Project>_<Tag>/<Target>_install/<Config>
file(GLOB _installed_third_parties
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
    "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
)
list(APPEND CMAKE_PREFIX_PATH ${_installed_third_parties} "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

find_package(QtWidgetsCommonLib HINTS ${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/QtWidgetsCommonLib NO_DEFAULT_PATHS)

if(QtWidgetsCommonLib_FOUND)
    message("QtWidgetsCommonLib found")
else()
    message("QtWidgetsCommonLib not found. Downloading and invoking cmake ..")
    build_third_party_project(
        false
        ${THIRD_PARTY_TARGET}
        https://github.com/Dingola/QtWidgetsCommonLib.git
        ${GIT_TAG}
        ${THIRD_PARTY_TARGET_DIR}
        ${CMAKE_BUILD_TYPE}
        ${CMAKE_ARGS}
    )

    # Re-scan third-party installs after the build to pick up dependencies that were just installed
    file(GLOB _installed_third_parties
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install/${CMAKE_BUILD_TYPE}"
        "${THIRD_PARTY_INCLUDE_DIR}/*/*_install"
    )
    list(APPEND CMAKE_PREFIX_PATH ${_installed_third_parties} "${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}")

    find_package(QtWidgetsCommonLib REQUIRED HINTS ${QtWidgetsCommonLib_INSTALL_ROOT}/${CMAKE_BUILD_TYPE}/lib/cmake/QtWidgetsCommonLib NO_DEFAULT_PATHS)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC QtWidgetsCommonLib)
