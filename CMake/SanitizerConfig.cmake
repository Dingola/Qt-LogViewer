# This CMake script configures the use of sanitizers for the build process.
# The user can specify the type of sanitizer to use via the SANITIZER_TYPE variable.
# Supported sanitizers vary by platform (MSVC, UNIX, others).
# If no sanitizer is specified or "none" is chosen, no sanitizers will be used.
#
# Supported sanitizer types and combinations:
# - none
# - address
# - leak
# - memory
# - thread
# - address_and_leak
# - address_and_memory
# - memory_and_leak
# - address_memory_and_leak
#
# Platform-specific notes:
# - MSVC supports: none, address
# - UNIX supports: all of the above

# Set the SANITIZER_TYPE variable with an empty default value and cache it as a string option
set(SANITIZER_TYPE "" CACHE STRING "Sanitizer type options")

# Define the allowed sanitizers based on the platform
if (MSVC)
    set(ALLOWED_SANITIZERS "none;address")
elseif (UNIX)
    set(ALLOWED_SANITIZERS "none;address;leak;memory;thread;address_and_leak;address_and_memory;memory_and_leak;address_memory_and_leak")
else()
    set(ALLOWED_SANITIZERS "none")
endif()

# Set the allowed values for the SANITIZER_TYPE cache variable
set_property(CACHE SANITIZER_TYPE PROPERTY STRINGS ${ALLOWED_SANITIZERS})

# Check if the specified SANITIZER_TYPE is in the list of allowed sanitizers
list(FIND ALLOWED_SANITIZERS "${SANITIZER_TYPE}" SANITIZER_INDEX)
if (SANITIZER_INDEX EQUAL -1 AND NOT SANITIZER_TYPE STREQUAL "")
    message(FATAL_ERROR "Unsupported sanitizer type: ${SANITIZER_TYPE}")
endif()

# If a valid sanitizer type is specified (not "none" and not empty), set the appropriate flags
if (NOT SANITIZER_TYPE STREQUAL "none" AND NOT SANITIZER_TYPE STREQUAL "")
    set(SANITIZER_FLAGS "")
    if (SANITIZER_TYPE STREQUAL "address")
        list(APPEND SANITIZER_FLAGS "-fsanitize=address")
    elseif (SANITIZER_TYPE STREQUAL "leak")
        list(APPEND SANITIZER_FLAGS "-fsanitize=leak")
    elseif (SANITIZER_TYPE STREQUAL "memory")
        list(APPEND SANITIZER_FLAGS "-fsanitize=memory")
    elseif (SANITIZER_TYPE STREQUAL "thread")
        list(APPEND SANITIZER_FLAGS "-fsanitize=thread")
    elseif (SANITIZER_TYPE STREQUAL "address_and_leak")
        list(APPEND SANITIZER_FLAGS "-fsanitize=address" "-fsanitize=leak")
    elseif (SANITIZER_TYPE STREQUAL "address_and_memory")
        list(APPEND SANITIZER_FLAGS "-fsanitize=address" "-fsanitize=memory")
    elseif (SANITIZER_TYPE STREQUAL "memory_and_leak")
        list(APPEND SANITIZER_FLAGS "-fsanitize=memory" "-fsanitize=leak")
    elseif (SANITIZER_TYPE STREQUAL "address_memory_and_leak")
        list(APPEND SANITIZER_FLAGS "-fsanitize=address" "-fsanitize=memory" "-fsanitize=leak")
    endif()

    # Append the sanitizer flags to the C and C++ compiler flags
    foreach (FLAG ${SANITIZER_FLAGS})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endforeach()

    # Ensure frame pointers are not omitted
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

    # If building in Debug mode, set optimization level to O1
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
    endif()

    message(STATUS "Building with sanitizers: ${SANITIZER_TYPE}")
else()
    message(STATUS "Building without sanitizers")
endif()
