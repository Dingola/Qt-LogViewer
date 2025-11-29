# SimpleQtLogger Integration
set(THIRD_PARTY_TARGET "SimpleQtLogger")
set(GIT_TAG "main")
set(PROJECT_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")
set(CMAKE_ARGS "-D ${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=SimpleQtLogger")

set(SimpleQtLogger_INSTALL_ROOT "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install")

set(SimpleQtLogger_INCLUDE_DIR "${SimpleQtLogger_INSTALL_ROOT}/include")
set(SimpleQtLogger_LIBRARY     "${SimpleQtLogger_INSTALL_ROOT}/lib/SimpleQtLogger.lib")

set(SimpleQtLogger_DIR "${SimpleQtLogger_INSTALL_ROOT}/lib/cmake/SimpleQtLogger" CACHE PATH "SimpleQtLogger package directory")

find_package(SimpleQtLogger HINTS "${SimpleQtLogger_DIR}")

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

    list(APPEND CMAKE_PREFIX_PATH "${SimpleQtLogger_INSTALL_ROOT}")

    find_package(SimpleQtLogger REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC SimpleQtLogger)
