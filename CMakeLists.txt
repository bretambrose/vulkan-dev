cmake_minimum_required (VERSION 3.0)

project(vulkan-dev)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_SIZE 64)
    add_definitions(-DPLATFORM_SIZE=64)
    add_definitions(-DX64)
    set(WINDOWS_EXTERNAL_DLL_TYPE x64)
else()
    set(PLATFORM_SIZE 32)
    add_definitions(-DPLATFORM_SIZE=32)
    set(WINDOWS_EXTERNAL_DLL_TYPE x86)
endif()

# external package setups
include(cmake/common-macros.cmake)
include(cmake/common-dependencies.cmake)

# default libraries to link in per-platform
set(PLATFORM_DEP_LIBS "")
#if(PLATFORM_WINDOWS)
#    set(PLATFORM_DEP_LIBS Shlwapi DbgHelp)
#elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
#    set(PLATFORM_DEP_LIBS pthread)
#endif()

set(ALL_PROJECT_BASE_LIBS vulkan-1 glfw3 ${PLATFORM_DEP_LIBS})

#targets
add_subdirectory(build-test)

