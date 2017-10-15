add_project(tutorial)

file(GLOB PROJECT_HEADERS
    "include/vulkan-dev/tutorial/*.h"
)

file(GLOB PROJECT_SOURCE
    "source/*.cpp"
)

file(GLOB PROJECT_UNIFIED_SRC
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCE}
)

if(WIN32)
    if(MSVC)
        source_group("Header Files" FILES ${PROJECT_HEADERS})
        source_group("Source Files" FILES ${PROJECT_SOURCE})
    endif(MSVC)
endif()

set(PROJECT_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include_directories(${PROJECT_INCLUDES})

add_executable(${PROJECT_NAME} ${PROJECT_UNIFIED_SRC})

target_link_libraries(${PROJECT_NAME} ip-render ip-core)


