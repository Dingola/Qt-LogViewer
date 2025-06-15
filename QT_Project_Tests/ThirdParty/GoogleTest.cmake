set(Third_Party_Target "googletest")
set(Git_Tag "v1.15.2")
set(Project_Directory_Name "${Third_Party_Target}_${Git_Tag}")
set(Third_Party_Target_Directory "${THIRD_PARTY_INCLUDE_DIR}/${Project_Directory_Name}")
set(GTEST_INCLUDE_DIR ${Third_Party_Target_Directory}/${Third_Party_Target}_install/${CMAKE_BUILD_TYPE}/include)
set(GTEST_LIBRARY ${Third_Party_Target_Directory}/${Third_Party_Target}_install/${CMAKE_BUILD_TYPE}/lib/gtest.lib)
set(GTEST_MAIN_LIBRARY ${Third_Party_Target_Directory}/${Third_Party_Target}_install/${CMAKE_BUILD_TYPE}/lib/gtest_main.lib)
set(GTest_DIR "")

find_package(GTest QUIET PATHS ${Third_Party_Target_Directory}/${Third_Party_Target}_install/${CMAKE_BUILD_TYPE}/lib/cmake/GTest NO_DEFAULT_PATHS)

if(GTest_FOUND)
    message("GTest found")
else()
    message("Gtest not found. Downloading and invoking cmake ..")
    build_third_party_project(
        false
        ${Third_Party_Target}
        https://github.com/google/googletest.git
        ${Git_Tag}
        ${Third_Party_Target_Directory}
		${CMAKE_BUILD_TYPE}
    )
endif()

add_subdirectory("${Third_Party_Target_Directory}/${Third_Party_Target}_src"
				 "${Third_Party_Target_Directory}/${Third_Party_Target}_build")

set_target_properties(gtest PROPERTIES FOLDER ThirdParty)
set_target_properties(gtest_main PROPERTIES FOLDER ThirdParty)
set_target_properties(gmock PROPERTIES FOLDER ThirdParty)
set_target_properties(gmock_main PROPERTIES FOLDER ThirdParty)

target_link_libraries(${PROJECT_NAME} PUBLIC gtest_main)
target_link_libraries(${PROJECT_NAME} PUBLIC gmock_main)