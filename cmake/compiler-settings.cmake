# stripped version of aws-cpp-sdk settings for familiarity
# shared lib settings preserved since I might want to use that someday

# Compiler recognition
SET(COMPILER_MSVC 0)
SET(COMPILER_GCC 0)
SET(COMPILER_CLANG 0)

# ToDo: extend as necessary and remove common assumptions
if(MSVC)
    SET(COMPILER_MSVC 1)
    add_definitions(-DMSVC)
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        SET(COMPILER_CLANG 1)
    else()
        SET(COMPILER_GCC 1)
    endif()
    SET(USE_GCC_FLAGS 1)
endif()

# Based on the FORCE_SHARED_CRT and BUILD_SHARED_LIBS options, make sure our compile/link flags bring in the right CRT library
# modified from gtest's version; while only the else clause is actually necessary, do both for completeness/future-proofing
macro(set_msvc_crt_settings)
    if(COMPILER_MSVC)
        foreach (var
                 CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                 CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            string(REPLACE "/MT" "/MD" ${var} "${${var}}")
        endforeach()
    endif()
endmacro()

set_msvc_crt_settings()

# cmake warnings
if(MSVC)
    #remove bogus errors at generation time if these variables haven't been manually set
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo")
    endif()
    
    if(NOT CMAKE_CXX_FLAGS_DEBUGOPT)
        set(CMAKE_CXX_FLAGS_DEBUGOPT "")
    endif()

    if(NOT CMAKE_EXE_LINKER_FLAGS_DEBUGOPT)
        set(CMAKE_EXE_LINKER_FLAGS_DEBUGOPT "")
    endif()

    if(NOT CMAKE_SHARED_LINKER_FLAGS_DEBUGOPT)
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUGOPT "")
    endif()
endif()

# compiler warning control
if(MSVC)
    # warnings as errors, max warning level (4)
    if(NOT CMAKE_CXX_FLAGS MATCHES "/WX")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    endif()

    # taken from http://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(USE_GCC_FLAGS)
    # max warning level, warnings are errors, turn off unused private field. We have one for an empty class.    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -Wextra -Wno-unused-value")

    if(COMPILER_CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
    endif()
endif()

# other compiler flags
if(MSVC)
    # special windows build options:
    #   debug info: pdbs with dlls, embedded in static libs
    #   release optimisations to purely focus on size, override debug info settings as necessary
    if(BUILD_SHARED_LIBS)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    else()
        if(CMAKE_CXX_FLAGS MATCHES "/Zi")
            string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        endif()
        if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/Zi")
            message(STATUS "Clearing pdb setting")
            string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        endif()

        # put Z7 in config-specific flags so we can strip from release if we're concerned about size
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Z7")
    endif()
elseif(USE_GCC_FLAGS)
    if(NOT BUILD_SHARED_LIBS)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif()

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lstdc++fs")
endif()
