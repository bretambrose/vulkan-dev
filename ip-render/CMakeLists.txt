add_project(ip-render)

file(GLOB RENDER_HEADERS "include/ip/render/*.h")
file(GLOB RENDER_VULKAN_HEADERS "include/ip/render/vulkan/*.h")

file(GLOB PROJECT_HEADERS
    ${RENDER_HEADERS}
    ${RENDER_VULKAN_HEADERS}
)

file(GLOB RENDER_SOURCE "source/*.cpp")
file(GLOB RENDER_VULKAN_SOURCE "source/vulkan/*.cpp")

file(GLOB PROJECT_SOURCE
    ${RENDER_SOURCE}
    ${RENDER_VULKAN_SOURCE}
)

file(GLOB PROJECT_UNIFIED_SRC
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCE}
)

if(WIN32)
    if(MSVC)
        source_group("Header Files" FILES ${RENDER_HEADERS})
        source_group("Header Files\\vulkan" FILES ${RENDER_VULKAN_HEADERS})
        source_group("Source Files" FILES ${RENDER_SOURCE})
        source_group("Source Files\\vulkan" FILES ${RENDER_VULKAN_SOURCE})
    endif(MSVC)
endif()

set(PROJECT_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include_directories(${PROJECT_INCLUDES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_UNIFIED_SRC})

target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLFW_INCLUDE_VULKAN)
target_link_libraries(${PROJECT_NAME} ${RENDERING_LIBS} ip-core)

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

