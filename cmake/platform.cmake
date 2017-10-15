
# Platform recognition
if(CMAKE_HOST_WIN32)
    SET(PLATFORM_WINDOWS 1)
    SET(TARGET_PLATFORM "WINDOWS")
elseif(CMAKE_HOST_UNIX)
    SET(PLATFORM_LINUX 1)
    SET(TARGET_PLATFORM "LINUX")
else()
    message(FATAL_ERROR "Unknown host OS; unable to determine platform compilation target")
endif()

message(STATUS "Building for ${TARGET_PLATFORM} platform")

# only usable in internal .cpp files
add_definitions(-DPLATFORM_${TARGET_PLATFORM})



