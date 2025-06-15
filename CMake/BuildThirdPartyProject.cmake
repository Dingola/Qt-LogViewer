# Function to build a third-party project from a Git repository.
#
# Parameters:
#   DONT_UPDATE_BUILD        - If set to 1, the project will not be updated if it already exists.
#   EXT_PROJ_TARGET          - The name of the external project target.
#   EXT_PROJ_GIT_REPO        - The Git repository URL of the external project.
#   EXT_PROJ_GIT_TAG         - The Git tag or branch to checkout.
#   THIRD_PARTY_DIR_PATH     - The directory path where the third-party project will be downloaded and built.
#   EXT_PROJ_BUILD_TYPE      - The build type (e.g., Debug, Release) for the external project.
#   ARGN                     - Additional CMake arguments to pass to the external project.
#
# Example usage:
#   build_third_party_project(
#       1
#       MyLib
#       https://github.com/example/MyLib.git
#       master
#       ${CMAKE_SOURCE_DIR}/third_party
#       Release
#       -DMYLIB_OPTION=ON
#   )
function (build_third_party_project 
	DONT_UPDATE_BUILD 
	EXT_PROJ_TARGET 
	EXT_PROJ_GIT_REPO 
	EXT_PROJ_GIT_TAG 
	THIRD_PARTY_DIR_PATH 
	EXT_PROJ_BUILD_TYPE
)
	# Create content of the CMakeLists which is used for downloading and building the third party lib
	set(CMAKELIST_CONTENT "
		cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})
		project(${EXT_PROJ_TARGET}_download)

		include(ExternalProject)
		ExternalProject_Add(${EXT_PROJ_TARGET}
			GIT_REPOSITORY           ${EXT_PROJ_GIT_REPO}
			GIT_TAG                  ${EXT_PROJ_GIT_TAG}
			SOURCE_DIR		         \"${THIRD_PARTY_DIR_PATH}/${EXT_PROJ_TARGET}_src\"
			BINARY_DIR               \"${THIRD_PARTY_DIR_PATH}/${EXT_PROJ_TARGET}_build\"
			INSTALL_DIR              \"${THIRD_PARTY_DIR_PATH}/${EXT_PROJ_TARGET}_install/${EXT_PROJ_BUILD_TYPE}\"
			CMAKE_GENERATOR          \"${CMAKE_GENERATOR}\"
			CMAKE_GENERATOR_PLATFORM \"${CMAKE_GENERATOR_PLATFORM}\"
			CMAKE_GENERATOR_TOOLSET  \"${CMAKE_GENERATOR_TOOLSET}\"
			CMAKE_GENERATOR_INSTANCE \"${CMAKE_GENERATOR_INSTANCE}\"
			UPDATE_DISCONNECTED ${DONT_UPDATE_BUILD}
			CMAKE_ARGS
				-DCMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY_DIR_PATH}/${EXT_PROJ_TARGET}_install/
				-DCMAKE_BUILD_TYPE=${EXT_PROJ_BUILD_TYPE}
				${ARGN}
		)
	")

	# Create download directory and copy CMakeList_Content to a CMakeLists.txt file
	set(EXT_PROJ_DOWNLOAD_DIR "${THIRD_PARTY_DIR_PATH}/${EXT_PROJ_TARGET}_download")
	file(MAKE_DIRECTORY ${EXT_PROJ_DOWNLOAD_DIR})
	file(WRITE "${EXT_PROJ_DOWNLOAD_DIR}/CMakeLists.txt" "${CMAKELIST_CONTENT}")
	
	# Configure and build of the third party lib
	execute_process(COMMAND ${CMAKE_COMMAND}
		-G "${CMAKE_GENERATOR}"
		-A "${CMAKE_GENERATOR_PLATFORM}"
		-T "${CMAKE_GENERATOR_TOOLSET}"
		.
		WORKING_DIRECTORY "${EXT_PROJ_DOWNLOAD_DIR}"
	)
	
	execute_process(COMMAND ${CMAKE_COMMAND}
		--build .
		--config ${EXT_PROJ_BUILD_TYPE}
		WORKING_DIRECTORY "${EXT_PROJ_DOWNLOAD_DIR}"
	)
endfunction()
