add_project(ip-core)

file(GLOB CORE_HEADERS "include/ip/core/*.h")
file(GLOB CORE_DEBUG_HEADERS "include/ip/core/debug/*.h")
file(GLOB CORE_LOGGING_HEADERS "include/ip/core/logging/*.h")
file(GLOB CORE_MEMORY_HEADERS "include/ip/core/memory/*.h")
file(GLOB CORE_MEMORY_STL_HEADERS "include/ip/core/memory/stl/*.h")
file(GLOB CORE_UTILS_HEADERS "include/ip/core/utils/*.h")

file(GLOB PROJECT_HEADERS
    ${CORE_HEADERS}
    ${CORE_DEBUG_HEADERS}
    ${CORE_LOGGING_HEADERS}
    ${CORE_MEMORY_HEADERS}
    ${CORE_MEMORY_STL_HEADERS}
    ${CORE_UTILS_HEADERS}
)

file(GLOB CORE_SOURCE "source/*.cpp")
file(GLOB CORE_DEBUG_SOURCE "source/debug/*.cpp")
file(GLOB CORE_LOGGING_SOURCE "source/logging/*.cpp")
file(GLOB CORE_MEMORY_SOURCE "source/memory/*.cpp")
file(GLOB CORE_UTILS_SOURCE "source/utils/*.cpp")

file(GLOB PROJECT_SOURCE
    ${CORE_SOURCE}
    ${CORE_DEBUG_SOURCE}
    ${CORE_LOGGING_SOURCE}
    ${CORE_MEMORY_SOURCE}
    ${CORE_UTILS_SOURCE}
)

file(GLOB PROJECT_UNIFIED_SRC
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCE}
)

if(WIN32)
    if(MSVC)
        source_group("Header Files" FILES ${CORE_HEADERS})
        source_group("Header Files\\debug" FILES ${CORE_DEBUG_HEADERS})
        source_group("Header Files\\logging" FILES ${CORE_LOGGING_HEADERS})
        source_group("Header Files\\memory" FILES ${CORE_MEMORY_HEADERS})
        source_group("Header Files\\memory\\stl" FILES ${CORE_MEMORY_STL_HEADERS})
        source_group("Header Files\\utils" FILES ${CORE_UTILS_HEADERS})
        source_group("Source Files" FILES ${CORE_SOURCE})
        source_group("Source Files\\debug" FILES ${CORE_DEBUG_SOURCE})
        source_group("Source Files\\logging" FILES ${CORE_LOGGING_SOURCE})
        source_group("Source Files\\memory" FILES ${CORE_MEMORY_SOURCE})
        source_group("Source Files\\utils" FILES ${CORE_UTILS_SOURCE})
    endif(MSVC)
endif()

set(PROJECT_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include_directories(${PROJECT_INCLUDES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_UNIFIED_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

