# sdk_path as a shorthand for multiple prefix paths
#
# assumes the sdks have been placed into a single directory and the paths massaged to be reasonable (no version names or intermediate directories)

if (DEFINED SDK_PATH)
  get_filename_component(SDK_PATH_FINAL "${SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
  if (EXISTS ${SDK_PATH_FINAL})
    #glm
    set(GLM_PATH "${SDK_PATH_FINAL}/glm")
    if (EXISTS ${GLM_PATH})
      message(STATUS "adding ${GLM_PATH} to search path")
      set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${GLM_PATH})
    endif()

    #glfw
    set(GLFW_PATH "${SDK_PATH_FINAL}/glfw")
    if (EXISTS ${GLFW_PATH})
      message(STATUS "adding ${GLFW_PATH} to search path")
      set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${GLFW_PATH})
    endif()

    #vulkan
    set(VULKAN_PATH "${SDK_PATH_FINAL}/VulkanSDK")
    if (EXISTS ${VULKAN_PATH})
      message(STATUS "adding ${VULKAN_PATH} to search path")
      set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VULKAN_PATH})
    endif()
  endif()
endif()

# vulkan headers
find_file(VULKAN_HEADER "Include/vulkan/vulkan.h")
if (EXISTS ${VULKAN_HEADER})
  get_filename_component(VULKAN_INCLUDES "${VULKAN_HEADER}" DIRECTORY)
  get_filename_component(VULKAN_INCLUDES "${VULKAN_INCLUDES}/.." REALPATH)
  include_directories(${VULKAN_INCLUDES})
  message(STATUS "Vulkan headers located at: ${VULKAN_INCLUDES}")
else()
  message(FATAL_ERROR "Could not find Vulkan header directory")
endif()

# vulkan libraries
find_file(VULKAN_LIBRARY "Lib/vulkan-1.lib")
if (EXISTS ${VULKAN_LIBRARY})
  get_filename_component(VULKAN_LIBRARIES "${VULKAN_LIBRARY}" DIRECTORY)
  link_directories(${VULKAN_LIBRARIES})
  message(STATUS "Vulkan libraries located at: ${VULKAN_LIBRARIES}")
else()
  message(FATAL_ERROR "Could not find Vulkan library directory")
endif()

# glfw headers
find_file(GLFW_HEADER "include/GLFW/glfw3.h")
if (EXISTS ${GLFW_HEADER})
  get_filename_component(GLFW_INCLUDES "${GLFW_HEADER}" DIRECTORY)
  get_filename_component(GLFW_INCLUDES "${GLFW_INCLUDES}/.." REALPATH)
  include_directories(${GLFW_INCLUDES})
  message(STATUS "Glfw headers located at: ${GLFW_INCLUDES}")
else()
  message(FATAL_ERROR "Could not find Glfw header directory")
endif()

# glfw libraries
find_file(GLFW_LIBRARY "lib-vc2015/glfw3.lib") # This sucks
if (EXISTS ${GLFW_LIBRARY})
  get_filename_component(GLFW_LIBRARIES "${GLFW_LIBRARY}" DIRECTORY)
  link_directories(${GLFW_LIBRARIES})
  message(STATUS "Glfw libraries located at: ${GLFW_LIBRARIES}")
else()
  message(FATAL_ERROR "Could not find Glfw library directory")
endif()

#glm headers
find_file(GLM_HEADER "glm/glm.hpp")
if (EXISTS ${GLM_HEADER})
  get_filename_component(GLM_INCLUDES "${GLM_HEADER}" DIRECTORY)
  get_filename_component(GLM_INCLUDES "${GLM_INCLUDES}/.." REALPATH)
  include_directories(${GLM_INCLUDES})
  message(STATUS "Glm headers located at: ${GLM_INCLUDES}")
else()
  message(FATAL_ERROR "Could not find glm header directory")
endif()
