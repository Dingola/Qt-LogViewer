cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)

project(${MAIN_PROJECT_NAME}_Performance LANGUAGES CXX VERSION ${MAIN_PROJECT_VERSION})

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS
  "*.cpp"
)

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_sources(${PROJECT_NAME}
  PRIVATE
    ${Sources}
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
target_link_libraries(${PROJECT_NAME} PUBLIC ${MAIN_PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/QT_Project/Headers)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE Psapi)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MAIN_PROJECT_NAME}_Performance)
