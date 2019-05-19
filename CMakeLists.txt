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
include(cmake/platform.cmake)
include(cmake/compiler-settings.cmake)
include(cmake/common-macros.cmake)
include(cmake/common-dependencies.cmake)

# default libraries to link in per-platform
set(PLATFORM_DEP_LIBS "")
if(PLATFORM_WINDOWS)
    # set(PLATFORM_DEP_LIBS Shlwapi DbgHelp)
elseif(PLATFORM_LINUX)
    set(PLATFORM_DEP_LIBS X11 dl pthread stdc++fs)
elseif(PLATFORM_APPLE)
    #set(PLATFORM_DEP_LIBS ??)
endif()

# GLU GL Xrandr Xext

if(PLATFORM_WINDOWS)
    set(RENDERING_LIBS glfw3 vulkan-1 ${PLATFORM_DEP_LIBS})
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    set(RENDERING_LIBS glfw3 vulkan ${PLATFORM_DEP_LIBS})
endif()


#targets
add_subdirectory(ip-core)
add_subdirectory(ip-render)
add_subdirectory(tutorial)
