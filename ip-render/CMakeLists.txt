add_project(ip-render)

file(GLOB RENDER_HEADERS "include/ip/render/*.h")
file(GLOB RENDER_MODEL_HEADERS "include/ip/render/model/*.h")
file(GLOB RENDER_SCENE_HEADERS "include/ip/render/scene/*.h")
file(GLOB RENDER_VULKAN_HEADERS "include/ip/render/vulkan/*.h")
file(GLOB RENDER_VULKAN_VISITORS_HEADERS "include/ip/render/vulkan/visitors/*.h")

file(GLOB PROJECT_HEADERS
    ${RENDER_HEADERS}
    ${RENDER_MODEL_HEADERS}
    ${RENDER_SCENE_HEADERS}
    ${RENDER_VULKAN_HEADERS}
    ${RENDER_VULKAN_VISITORS_HEADERS}
)

file(GLOB RENDER_SOURCE "source/*.cpp")
file(GLOB RENDER_MODEL_SOURCE "source/model/*.cpp")
file(GLOB RENDER_SCENE_SOURCE "source/scene/*.cpp")
file(GLOB RENDER_VULKAN_SOURCE "source/vulkan/*.cpp")
file(GLOB RENDER_VULKAN_VISITORS_SOURCE "source/vulkan/visitors/*.cpp")

file(GLOB PROJECT_SOURCE
    ${RENDER_SOURCE}
    ${RENDER_MODEL_SOURCE}
    ${RENDER_SCENE_SOURCE}
    ${RENDER_VULKAN_SOURCE}
    ${RENDER_VULKAN_VISITORS_SOURCE}
)

file(GLOB PROJECT_UNIFIED_SRC
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCE}
)

if(WIN32)
    if(MSVC)
        source_group("Header Files" FILES ${RENDER_HEADERS})
        source_group("Header Files\\model" FILES ${RENDER_MODEL_HEADERS})
        source_group("Header Files\\scene" FILES ${RENDER_SCENE_HEADERS})
        source_group("Header Files\\vulkan" FILES ${RENDER_VULKAN_HEADERS})
        source_group("Header Files\\vulkan\\visitors" FILES ${RENDER_VULKAN_VISITORS_HEADERS})
        source_group("Source Files" FILES ${RENDER_SOURCE})
        source_group("Source Files\\model" FILES ${RENDER_MODEL_SOURCE})
        source_group("Source Files\\scene" FILES ${RENDER_SCENE_SOURCE})
        source_group("Source Files\\vulkan" FILES ${RENDER_VULKAN_SOURCE})
        source_group("Source Files\\vulkan\\visitors" FILES ${RENDER_VULKAN_VISITORS_SOURCE})
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

