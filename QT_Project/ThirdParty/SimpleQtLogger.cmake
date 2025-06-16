# SimpleQtLogger Integration
set(THIRD_PARTY_TARGET "SimpleQtLogger")
set(GIT_TAG "main")
set(PROJECT_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJECT_DIR_NAME}")
set(CMAKE_ARGS "-D ${THIRD_PARTY_TARGET}_BUILD_TARGET_TYPE:STRING=static_library -D MAIN_PROJECT_NAME:STRING=SimpleQtLogger")
set(SimpleQtLogger_INCLUDE_DIR ${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install/include)
set(SimpleQtLogger_LIBRARY ${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install/lib/SimpleQtLogger.lib)
set(SimpleQtLogger_DIR "")

find_package(SimpleQtLogger HINTS ${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install/lib/cmake/SimpleQtLogger NO_DEFAULT_PATHS)

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
	
	find_package(SimpleQtLogger REQUIRED HINTS ${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install/lib/cmake/SimpleQtLogger NO_DEFAULT_PATHS)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC SimpleQtLogger)
