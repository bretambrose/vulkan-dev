#include <ip/render/vulkan/VulkanRenderer.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>

#include <ip/core/debug/IPException.h>
#include <ip/core/logging/LogSystem.h>
#include <ip/core/memory/stl/Set.h>
#include <ip/core/UnreferencedParam.h>
#include <ip/core/utils/FileUtils.h>

#include <ip/render/DisplayMode.h>
#include <ip/render/GlfwError.h>
#include <ip/render/RenderDebugLevel.h>
#include <ip/render/vulkan/ScopedVulkanShader.h>
#include <ip/render/vulkan/VulkanDeviceProperties.h>


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                                    VkDebugReportObjectTypeEXT objType,
                                                    uint64_t obj,
                                                    size_t location,
                                                    int32_t code,
                                                    const char* layerPrefix,
                                                    const char* msg,
                                                    void* userData) 
{
    IP_UNREFERENCED_PARAM(flags);
    IP_UNREFERENCED_PARAM(objType);
    IP_UNREFERENCED_PARAM(obj);
    IP_UNREFERENCED_PARAM(location);
    IP_UNREFERENCED_PARAM(code);
    IP_UNREFERENCED_PARAM(layerPrefix);
    IP_UNREFERENCED_PARAM(userData);

    LOG_DEBUG("Vulkan validation layer: ", msg);

    return VK_FALSE;
}

namespace IP
{
namespace Render
{

static const char* CREATE_DEBUG_CALLBACK_PROCEDURE_NAME = "vkCreateDebugReportCallbackEXT";
static const char* DESTROY_DEBUG_CALLBACK_PROCEDURE_NAME = "vkDestroyDebugReportCallbackEXT";

static const std::vector<const char *> s_debugValidationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

bool ShouldUseValidationLayers(const RendererConfig& config)
{
    return config.m_debugLevel == RenderDebugLevel::Debug;
}

VulkanRenderer::VulkanRenderer() :
    m_config(),
    m_window(nullptr),
    m_vulkanInstance(nullptr),
    m_validationCallback(nullptr),
    m_physicalDevice(VK_NULL_HANDLE),
    m_logicalDevice(VK_NULL_HANDLE),
    m_surface(VK_NULL_HANDLE),
    m_swapChain(VK_NULL_HANDLE),
    m_swapChainImageViews(),
    m_presentationQueue(VK_NULL_HANDLE),
    m_graphicsQueue(VK_NULL_HANDLE),
    m_swapChainImages(),
    m_renderPass(VK_NULL_HANDLE),
    m_pipelineLayout(VK_NULL_HANDLE),
    m_graphicsPipeline(VK_NULL_HANDLE),
    m_swapChainFramebuffers(),
    m_commandPool(VK_NULL_HANDLE),
    m_commandBuffers(),
    m_imageAvailableSemaphore(VK_NULL_HANDLE),
    m_renderFinishedSemaphore(VK_NULL_HANDLE),
    m_vulkanExtensionNames(),
    m_deviceExtensionNames(),
    m_validationLayerNames(),
    m_selectedDeviceProperties(),
    m_swapSurfaceFormat(),
    m_swapPresentationMode(),
    m_swapExtents(),
    m_glfwTerminate(false)
{
    glfwSetErrorCallback(GlfwErrorTracker::GlfwErrorCallback);

    int initResult = glfwInit();
    if (initResult != GLFW_TRUE)
    {
        THROW_IP_EXCEPTION("Failure initializing glfw.  Error code: ", initResult);
    }

    m_glfwTerminate = true;
}

VulkanRenderer::~VulkanRenderer()
{
    Shutdown();
}

void VulkanRenderer::FillInConfig()
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (m_config.m_windowHeight == 0)
    {
        m_config.m_windowHeight = mode->height;
    }

    if (m_config.m_windowWidth == 0)
    {
        m_config.m_windowWidth = mode->width;
    }

    if (m_config.m_redBits == 0)
    {
        m_config.m_redBits = static_cast<uint8_t>(mode->redBits);
    }

    if (m_config.m_greenBits == 0)
    {
        m_config.m_greenBits = static_cast<uint8_t>(mode->greenBits);
    }

    if (m_config.m_blueBits == 0)
    {
        m_config.m_blueBits = static_cast<uint8_t>(mode->blueBits);
    }

    if (m_config.m_refreshRate == 0)
    {
        m_config.m_refreshRate = mode->refreshRate;
    }
}

void VulkanRenderer::Initialize(const RendererConfig& config)
{
    m_config = config;
    FillInConfig();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RED_BITS, m_config.m_redBits);
    glfwWindowHint(GLFW_GREEN_BITS, m_config.m_greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, m_config.m_blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, m_config.m_refreshRate);

    m_window = glfwCreateWindow(m_config.m_windowWidth, m_config.m_windowHeight, m_config.m_windowName.c_str(), m_config.m_windowed ? nullptr : glfwGetPrimaryMonitor(), nullptr);
    if (m_window == nullptr)
    {
        THROW_IP_EXCEPTION("Glfw failed to build window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, VulkanRenderer::OnWindowResized);

    InitializeRenderer();
}

void VulkanRenderer::OnWindowResized(GLFWwindow* window, int width, int height) 
{
    if (width == 0 || height == 0) 
    {
        return;
    }

    VulkanRenderer* renderer = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    renderer->ResetSwapChainRelatedResources();
}

void VulkanRenderer::Shutdown()
{
    CleanupRenderer();

    if(m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    if (m_glfwTerminate)
    {
        glfwTerminate();
        m_glfwTerminate = false;
    }
}

bool VulkanRenderer::HandleInput()
{
    if (glfwWindowShouldClose(m_window)) 
    {
        return false;
    }

    glfwPollEvents();

    return true;
}

void VulkanRenderer::InitializeRenderer()
{
    InitializeVulkanInstance();
    InitializeValidationCallback();
    InitializeSurface();
    InitializeDevice();
    InitializeSwapChain();
    InitializeSwapChainImageViews();
    InitializeRenderPass();
    InitializeGraphicsPipeline();
    InitializeFramebuffers();
    InitializeCommandPool();
    InitializeCommandBuffers();
    InitializeSynchronization();
}

void VulkanRenderer::ResetSwapChainRelatedResources()
{
    vkDeviceWaitIdle(m_logicalDevice);

    CleanupSwapChainRelatedResources();

    InitializeSwapChain();
    InitializeSwapChainImageViews();
    InitializeRenderPass();
    InitializeGraphicsPipeline();
    InitializeFramebuffers();
    InitializeCommandBuffers();
}

void VulkanRenderer::CleanupSwapChainRelatedResources()
{
    CleanupFramebuffers();

    if (!m_commandBuffers.empty())
    {
        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    if (m_graphicsPipeline)
    {
        vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
        m_graphicsPipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout)
    {
        vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_renderPass)
    {
        vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    CleanupSwapChainImageViews();

    if (m_swapChain)
    {
        vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::InitializeVulkanInstance()
{
    BuildValidationLayerSet();
    IP::Vector<const char *> rawLayerNames;
    std::for_each(m_validationLayerNames.cbegin(), m_validationLayerNames.cend(), [&](const IP::String& name){ rawLayerNames.push_back(name.c_str()); });

    BuildVulkanExtensionSet();
    IP::Vector<const char *> rawExtensionNames;
    std::for_each(m_vulkanExtensionNames.cbegin(), m_vulkanExtensionNames.cend(), [&](const IP::String& name){ rawExtensionNames.push_back(name.c_str()); });

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "IPRender-based Application";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "None";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(rawExtensionNames.size());
    if (!rawExtensionNames.empty())
    {
        createInfo.ppEnabledExtensionNames = rawExtensionNames.data();
    }

    createInfo.enabledLayerCount = static_cast<uint32_t>(rawLayerNames.size());
    if (!rawLayerNames.empty())
    {
        createInfo.ppEnabledLayerNames = rawLayerNames.data();
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Failed to initialize Vulkan instance, error code: ", (int)result);
    }
}

void VulkanRenderer::CleanupRenderer()
{
    if (m_logicalDevice)
    {
        vkDeviceWaitIdle(m_logicalDevice);
    }

    if (m_renderFinishedSemaphore)
    {
        vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (m_imageAvailableSemaphore)
    {
        vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    CleanupSwapChainRelatedResources();

    if (m_commandPool)
    {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    if (m_logicalDevice)
    {
        vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = VK_NULL_HANDLE;
    }

    if (m_surface)
    {
        vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_validationCallback)
    {
        CleanupValidationCallback();
        m_validationCallback = nullptr;
    }

    if (m_vulkanInstance)
    {
        vkDestroyInstance(m_vulkanInstance, nullptr);
        m_vulkanInstance = nullptr;
    }
}

void VulkanRenderer::BuildVulkanExtensionSet()
{
    // query what's available
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    IP::Vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // make an easy to search set
    IP::Set<IP::String> presentExtensions;
    for(const auto& extensionProperties : extensions)
    {
        presentExtensions.insert(IP::String(extensionProperties.extensionName));
    }

    // build and check required extensions, throw exception on missing
    IP::Vector<IP::String> requiredExtensions;

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for(uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        requiredExtensions.push_back(IP::String(glfwExtensions[i]));
    }

    if (ShouldUseValidationLayers(m_config))
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    for (const auto& requiredExtension : requiredExtensions)
    {
        if (presentExtensions.find(requiredExtension) == presentExtensions.cend())
        {
            THROW_IP_EXCEPTION("Vulkan drivers missing required extension: ", requiredExtension);
        }

        m_vulkanExtensionNames.push_back(requiredExtension);
    }

    // build and check optional extensions, filter out on missing
    IP::Vector<IP::String> optionalExtensions = GetOptionalVulkanExtensions();
    for (const auto& extensionName : optionalExtensions)
    {
        if (presentExtensions.find(extensionName) != presentExtensions.cend())
        {
            m_vulkanExtensionNames.push_back(extensionName);
        }
    }
}

IP::Vector<IP::String> VulkanRenderer::GetOptionalVulkanExtensions() const
{
    IP::Vector<IP::String> optionalExtensions;

    return optionalExtensions;
}

void VulkanRenderer::BuildValidationLayerSet()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    IP::Vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    IP::Set<IP::String> layerSet;
    for (const auto& layer : availableLayers)
    {
        layerSet.insert(IP::String(layer.layerName));
    }

    // build and check required validation layers, throw exception on missing
    IP::Vector<IP::String> requiredLayers;
    if (ShouldUseValidationLayers(m_config))
    {
        std::copy(s_debugValidationLayers.cbegin(), s_debugValidationLayers.cend(), std::back_inserter(requiredLayers));
    }

    for(const auto& requiredLayer : requiredLayers)
    {
        if (layerSet.find(requiredLayer) == layerSet.cend())
        {
            THROW_IP_EXCEPTION("Vulkan drivers missing required validation layer: ", requiredLayer);
        }

        m_validationLayerNames.push_back(requiredLayer);        
    }

    // build and check optional layers, filter out on missing
    IP::Vector<IP::String> optionalLayers = GetOptionalValidationLayers();
    for (const auto& layerName : optionalLayers)
    {
        if (layerSet.find(layerName) != layerSet.cend())
        {
            m_validationLayerNames.push_back(layerName);
        }
    }
}

IP::Vector<IP::String> VulkanRenderer::GetOptionalValidationLayers() const
{
    IP::Vector<IP::String> optionalLayers;

    return optionalLayers;
}

void VulkanRenderer::InitializeValidationCallback()
{
    if (m_validationLayerNames.empty())
    {
        return;
    }

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    auto createCallbackFunc = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(m_vulkanInstance, CREATE_DEBUG_CALLBACK_PROCEDURE_NAME);
    if (createCallbackFunc == nullptr)
    {
        THROW_IP_EXCEPTION("Could not find required extension function: ", CREATE_DEBUG_CALLBACK_PROCEDURE_NAME);
    }

    VkResult result = createCallbackFunc(m_vulkanInstance, &createInfo, nullptr, &m_validationCallback);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Failed to create validation callback with error: ", (int32_t) result);
    }
}

void VulkanRenderer::CleanupValidationCallback()
{
    auto destroyFunc = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(m_vulkanInstance, DESTROY_DEBUG_CALLBACK_PROCEDURE_NAME);
    if (destroyFunc != nullptr) 
    {
        destroyFunc(m_vulkanInstance, m_validationCallback, nullptr);
    }
}

void VulkanRenderer::InitializeDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        THROW_IP_EXCEPTION("Vulkan: no available physical devices");
    }

    IP::Vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, physicalDevices.data());

    int32_t bestDeviceScore = 0;
    VkPhysicalDevice bestDevice = nullptr;
    VulkanDeviceProperties bestDeviceProperties;
    for(const auto& device : physicalDevices)
    {
        VulkanDeviceProperties deviceProperties;
        int32_t score = ScorePhysicalDevice(device, deviceProperties);
        if (score > bestDeviceScore) 
        {
            bestDeviceScore = score;
            bestDevice = device;
            bestDeviceProperties = deviceProperties;
        }
    }

    if (bestDevice == nullptr)
    {
        THROW_IP_EXCEPTION("Vulkan: no physical devices meet application requirements");
    }

    m_physicalDevice = bestDevice;

    float queuePriority = 1.0f;
    IP::Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(bestDeviceProperties.m_graphicsQueueFamilyIndex != bestDeviceProperties.m_presentationQueueFamilyIndex ? 2 : 1);
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueFamilyIndex = bestDeviceProperties.m_graphicsQueueFamilyIndex;
    deviceQueueCreateInfos[0].queueCount = 1;
    deviceQueueCreateInfos[0].pQueuePriorities = &queuePriority;
    if (bestDeviceProperties.m_graphicsQueueFamilyIndex != bestDeviceProperties.m_presentationQueueFamilyIndex)
    {
        deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfos[1].queueFamilyIndex = bestDeviceProperties.m_presentationQueueFamilyIndex;
        deviceQueueCreateInfos[1].queueCount = 1;
        deviceQueueCreateInfos[1].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (!BuildDeviceExtensionSet(m_physicalDevice, m_deviceExtensionNames))
    {
        THROW_IP_EXCEPTION("Vulkan: selected device does not have support for required extensions");
    }

    IP::Vector<const char *> deviceExtensions;
    std::for_each(m_deviceExtensionNames.cbegin(), m_deviceExtensionNames.cend(), [&](const IP::String& name){deviceExtensions.push_back(name.c_str());});

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    if (ShouldUseValidationLayers(m_config))
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(s_debugValidationLayers.size());
        createInfo.ppEnabledLayerNames = s_debugValidationLayers.data();
    } 

    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice);
    if (result != VK_SUCCESS) 
    {
        THROW_IP_EXCEPTION("Vulkan - could not create logical device");
    }

    vkGetDeviceQueue(m_logicalDevice, bestDeviceProperties.m_presentationQueueFamilyIndex, 0, &m_presentationQueue);
    if (m_presentationQueue == VK_NULL_HANDLE)
    {
        THROW_IP_EXCEPTION("Vulkan - could not fetch presentation queue from logical device");
    }

    vkGetDeviceQueue(m_logicalDevice, bestDeviceProperties.m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
    if (m_graphicsQueue == VK_NULL_HANDLE)
    {
        THROW_IP_EXCEPTION("Vulkan - could not fetch graphics queue");
    }
}

void VulkanRenderer::ExtractPhysicalDeviceProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const
{
    ExtractQueueFamilyProperties(device, deviceProperties);
    ExtractSwapChainProperties(device, deviceProperties);
}

void VulkanRenderer::ExtractQueueFamilyProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const
{
    // Queues and queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    IP::Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for(uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
        if(queueFamily.queueCount > 0)
        {
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                deviceProperties.m_graphicsQueueFamilyIndex = i;
            }
            
            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentationSupport);
            if (presentationSupport)
            {
                deviceProperties.m_presentationQueueFamilyIndex = i;
            }
        }

        IP::Vector<IP::String> supportedExtensions;
        deviceProperties.m_supportsRequiredExtensions = BuildDeviceExtensionSet(device, supportedExtensions);

        // if we've found a family with a single queue that satisfies all of our needs then stop, otherwise keep looking
        if (deviceProperties.MeetsOptimumQueueRequirements())
        {
            break;
        }
    }
}

void VulkanRenderer::ExtractSwapChainProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const
{
    // Swap chain related
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &deviceProperties.m_surfaceCapabilities);
    if (result != VK_SUCCESS)
    {
        // not fatal, but should log
        return;
    }

    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        // not fatal, but should log
        return;
    }

    if (surfaceFormatCount > 0) 
    {
        deviceProperties.m_surfaceFormats.resize(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &surfaceFormatCount, deviceProperties.m_surfaceFormats.data());
        if (result != VK_SUCCESS)
        {
            return;
        }
    }

    uint32_t presentationModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationModeCount, nullptr);
    if (result != VK_SUCCESS)
    {
        // not fatal, but should log
        return;
    }

    if (presentationModeCount > 0) {
        deviceProperties.m_presentationModes.resize(presentationModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationModeCount, deviceProperties.m_presentationModes.data());
        if (result != VK_SUCCESS)
        {
            // not fatal, but should log
            return;
        }
    }
}

int32_t VulkanRenderer::ScorePhysicalDevice(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const
{
    ExtractPhysicalDeviceProperties(device, deviceProperties);

    if (deviceProperties.MeetsMinimumRequirements())
    {
        return 1;
    }

    return 0;
}

void VulkanRenderer::InitializeSurface()
{
    VkResult result = glfwCreateWindowSurface(m_vulkanInstance, m_window, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        THROW_IP_EXCEPTION("Failed to create window surface");
    }
}

bool VulkanRenderer::BuildDeviceExtensionSet(VkPhysicalDevice device, IP::Vector<IP::String>& extensions) const
{
    // query what's available
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    IP::Vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // make an easy to search set
    IP::Set<IP::String> presentExtensions;
    std::for_each(availableExtensions.cbegin(), availableExtensions.cend(), [&](const VkExtensionProperties& properties){ presentExtensions.insert(IP::String(properties.extensionName)); });

    // build and check required extensions, throw exception on missing
    IP::Vector<IP::String> requiredExtensions = GetRequiredDeviceExtensions();
    for (const auto& requiredExtension : requiredExtensions)
    {
        if (presentExtensions.find(requiredExtension) == presentExtensions.cend())
        {
            return false;
        }

        extensions.push_back(requiredExtension);
    }

    // build and check optional extensions, filter out on missing
    IP::Vector<IP::String> optionalExtensions = GetOptionalDeviceExtensions();
    for (const auto& extensionName : optionalExtensions)
    {
        if (presentExtensions.find(extensionName) != presentExtensions.cend())
        {
            extensions.push_back(extensionName);
        }
    }

    return true;
}

IP::Vector<IP::String> VulkanRenderer::GetRequiredDeviceExtensions() const
{
    IP::Vector<IP::String> requiredExtensions;
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return requiredExtensions;
}

IP::Vector<IP::String> VulkanRenderer::GetOptionalDeviceExtensions() const
{
    IP::Vector<IP::String> optionalExtensions;

    return optionalExtensions;
}

VkSurfaceFormatKHR VulkanRenderer::SelectSwapSurfaceFormat() const
{
    if (m_selectedDeviceProperties.m_surfaceFormats.size() == 1 && m_selectedDeviceProperties.m_surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& surfaceFormat : m_selectedDeviceProperties.m_surfaceFormats) 
    {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surfaceFormat;
        }
    }

    return m_selectedDeviceProperties.m_surfaceFormats[0];
}

VkPresentModeKHR VulkanRenderer::SelectSwapPresentationMode() const
{
    for (const auto& presentationMode : m_selectedDeviceProperties.m_presentationModes) {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentationMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::SelectSwapExtent() const
{
    if (m_selectedDeviceProperties.m_surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return m_selectedDeviceProperties.m_surfaceCapabilities.currentExtent;
    }

    int windowWidth;
    int windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);

    VkExtent2D actualExtent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) };

    actualExtent.width = std::max(m_selectedDeviceProperties.m_surfaceCapabilities.minImageExtent.width, std::min(m_selectedDeviceProperties.m_surfaceCapabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(m_selectedDeviceProperties.m_surfaceCapabilities.minImageExtent.height, std::min(m_selectedDeviceProperties.m_surfaceCapabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void VulkanRenderer::InitializeSwapChain()
{
    ExtractPhysicalDeviceProperties(m_physicalDevice, m_selectedDeviceProperties);

    m_swapSurfaceFormat = SelectSwapSurfaceFormat();
    m_swapPresentationMode = SelectSwapPresentationMode();
    m_swapExtents = SelectSwapExtent();

    uint32_t imageCount = m_selectedDeviceProperties.m_surfaceCapabilities.minImageCount + 1;
    uint32_t maxImages = m_selectedDeviceProperties.m_surfaceCapabilities.maxImageCount;
    if (maxImages > 0) 
    {
        imageCount = std::min(maxImages, imageCount);
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_swapSurfaceFormat.format;
    createInfo.imageColorSpace = m_swapSurfaceFormat.colorSpace;
    createInfo.imageExtent = m_swapExtents;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {(uint32_t) m_selectedDeviceProperties.m_graphicsQueueFamilyIndex, (uint32_t) m_selectedDeviceProperties.m_presentationQueueFamilyIndex};

    if (m_selectedDeviceProperties.m_graphicsQueueFamilyIndex != m_selectedDeviceProperties.m_presentationQueueFamilyIndex) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; 
        createInfo.pQueueFamilyIndices = nullptr; 
    }

    createInfo.preTransform = m_selectedDeviceProperties.m_surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = m_swapPresentationMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Vulkan: Failed to build swap chain");
    }

    uint32_t swapImageCount = 0;
    result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &swapImageCount, nullptr);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Vulkan: Failed to retrieve swap chain image count");
    }

    m_swapChainImages.resize(swapImageCount);
    result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &swapImageCount, m_swapChainImages.data());
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Vulkan: Failed to retrieve swap chain image set");
    }
}

void VulkanRenderer::InitializeSwapChainImageViews()
{
    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapSurfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &imageView);
        if (result != VK_SUCCESS)
        {
            THROW_IP_EXCEPTION("Vulkan: Unable to create image view for swap chain image");
        }

        m_swapChainImageViews.push_back(imageView);
    }
}

void VulkanRenderer::CleanupSwapChainImageViews()
{
    for(const auto& imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_logicalDevice, imageView, nullptr);
    }

    m_swapChainImageViews.clear();
}

void VulkanRenderer::InitializeRenderPass()
{
    VkAttachmentDescription colorAttachmentConfig = {};
    colorAttachmentConfig.format = m_swapSurfaceFormat.format;
    colorAttachmentConfig.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentConfig.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentConfig.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentConfig.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentConfig.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentConfig.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentConfig.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRefConfig = {};
    colorAttachmentRefConfig.attachment = 0;
    colorAttachmentRefConfig.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassConfig = {};
    subpassConfig.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassConfig.colorAttachmentCount = 1;
    subpassConfig.pColorAttachments = &colorAttachmentRefConfig;

    VkSubpassDependency subpassDependencyConfig = {};
    subpassDependencyConfig.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencyConfig.dstSubpass = 0;
    subpassDependencyConfig.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencyConfig.srcAccessMask = 0;
    subpassDependencyConfig.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencyConfig.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachmentConfig;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassConfig;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDependencyConfig;

    VkResult result = vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass);
    if (result != VK_SUCCESS) 
    {
        THROW_IP_EXCEPTION("Could not create render pass");
    }
}

void VulkanRenderer::InitializeGraphicsPipeline()
{
    ScopedVulkanShader vertexShader(IP::FileUtils::LoadFileData("resources/shaders/hard_coded_triangle_vert.spv"), m_logicalDevice);
    ScopedVulkanShader fragmentShader(IP::FileUtils::LoadFileData("resources/shaders/hard_coded_triangle_frag.spv"), m_logicalDevice);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.pName = "main";
    vertexShaderStageInfo.module = vertexShader.GetShaderModule();

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.pName = "main";
    fragmentShaderStageInfo.module = fragmentShader.GetShaderModule();

    VkPipelineShaderStageCreateInfo shaderStages[] = { 
        vertexShaderStageInfo, 
        fragmentShaderStageInfo 
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapExtents.width;
    viewport.height = (float)m_swapExtents.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapExtents;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;  // true => GPU feature enable
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f; // != 1 => widelines GPU feature enable
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;   // True => GPU feature enable
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // skip depth/stencil

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutConfig = {};
    pipelineLayoutConfig.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result = vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutConfig, nullptr, &m_pipelineLayout);
    if (result != VK_SUCCESS) 
    {
        THROW_IP_EXCEPTION("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;

    result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);
    if (result != VK_SUCCESS) 
    {
        THROW_IP_EXCEPTION("Failed to create graphics pipeline");
    }
}

void VulkanRenderer::InitializeFramebuffers()
{
    for (size_t i = 0; i < m_swapChainImageViews.size(); ++i) 
    {
        VkImageView attachments[] = {
            m_swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferConfig = {};
        framebufferConfig.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferConfig.renderPass = m_renderPass;
        framebufferConfig.attachmentCount = 1;
        framebufferConfig.pAttachments = attachments;
        framebufferConfig.width = m_swapExtents.width;
        framebufferConfig.height = m_swapExtents.height;
        framebufferConfig.layers = 1;

        VkFramebuffer frameBuffer = VK_NULL_HANDLE;
        VkResult result = vkCreateFramebuffer(m_logicalDevice, &framebufferConfig, nullptr, &frameBuffer);
        if (result != VK_SUCCESS) 
        {
            THROW_IP_EXCEPTION("Failed to create framebuffer ", i);
        }

        m_swapChainFramebuffers.push_back(frameBuffer);
    }
}

void VulkanRenderer::CleanupFramebuffers()
{
    for (const auto& buffer : m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_logicalDevice, buffer, nullptr);
    }

    m_swapChainFramebuffers.clear();
}

void VulkanRenderer::InitializeCommandPool()
{
    VkCommandPoolCreateInfo commandPoolConfig = {};
    commandPoolConfig.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolConfig.queueFamilyIndex = m_selectedDeviceProperties.m_graphicsQueueFamilyIndex;
    commandPoolConfig.flags = 0; // Once dynamic scene graph and per-frame recreation, move to VK_COMMAND_POOL_CREATE_TRANSIENT_BIT

    VkResult result = vkCreateCommandPool(m_logicalDevice, &commandPoolConfig, nullptr, &m_commandPool);
    if (result != VK_SUCCESS) 
    {
        THROW_IP_EXCEPTION("Failed to create command pool");
    }
}

void VulkanRenderer::InitializeCommandBuffers()
{
    for (size_t i = 0; i < m_swapChainFramebuffers.size(); ++i)
    {
        VkCommandBufferAllocateInfo commandBufferAllocConfig = {};
        commandBufferAllocConfig.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocConfig.commandPool = m_commandPool;
        commandBufferAllocConfig.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocConfig.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult result = vkAllocateCommandBuffers(m_logicalDevice, &commandBufferAllocConfig, &commandBuffer);
        if (result != VK_SUCCESS) 
        {
            THROW_IP_EXCEPTION("Failed to create command buffer ", i);
        }

        m_commandBuffers.push_back(commandBuffer);

        VkCommandBufferBeginInfo commandBufferBeginConfig = {};
        commandBufferBeginConfig.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginConfig.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginConfig);
        if (result != VK_SUCCESS)
        {
            THROW_IP_EXCEPTION("Failed to begin recording command buffer ", i);
        }

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

        VkRenderPassBeginInfo beginRenderPassConfig = {};
        beginRenderPassConfig.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginRenderPassConfig.renderPass = m_renderPass;
        beginRenderPassConfig.framebuffer = m_swapChainFramebuffers[i];
        beginRenderPassConfig.renderArea.offset = { 0, 0 };
        beginRenderPassConfig.renderArea.extent = m_swapExtents;
        beginRenderPassConfig.clearValueCount = 1;
        beginRenderPassConfig.pClearValues = &clearColor;
        
        vkCmdBeginRenderPass(commandBuffer, &beginRenderPassConfig, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        if (result != VK_SUCCESS) {
            THROW_IP_EXCEPTION("Failed to end recording command buffer ", i);
        }
    }
}

void VulkanRenderer::InitializeSynchronization()
{
    VkSemaphoreCreateInfo semaphoreConfig = {};
    semaphoreConfig.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(m_logicalDevice, &semaphoreConfig, nullptr, &m_imageAvailableSemaphore);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Failed to create image available semaphore, result ", (int32_t)result);
    }

    result = vkCreateSemaphore(m_logicalDevice, &semaphoreConfig, nullptr, &m_renderFinishedSemaphore);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Failed to create render finished semaphore, result ", (int32_t)result);
    }
}

bool VulkanRenderer::HandleRenderingError(VkResult result)
{
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        ResetSwapChainRelatedResources();
        return true;
    }
    
    return false;
}

bool VulkanRenderer::RenderFrame()
{
    if (ShouldUseValidationLayers(m_config))
    {
        vkQueueWaitIdle(m_presentationQueue);
    }

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS)
    {
        return HandleRenderingError(result);
    }

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitConfig = {};
    submitConfig.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitConfig.waitSemaphoreCount = 1;
    submitConfig.pWaitSemaphores = waitSemaphores;
    submitConfig.pWaitDstStageMask = waitStages;
    submitConfig.commandBufferCount = 1;
    submitConfig.pCommandBuffers = &m_commandBuffers[imageIndex];
    submitConfig.signalSemaphoreCount = 1;
    submitConfig.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(m_graphicsQueue, 1, &submitConfig, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        return true; // Log, no exceptions inside rendering
    }

    VkSwapchainKHR swapChains[] = { m_swapChain };

    VkPresentInfoKHR presentConfig = {};
    presentConfig.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentConfig.waitSemaphoreCount = 1;
    presentConfig.pWaitSemaphores = signalSemaphores;
    presentConfig.swapchainCount = 1;
    presentConfig.pSwapchains = swapChains;
    presentConfig.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_presentationQueue, &presentConfig);

    if (result != VK_SUCCESS)
    {
        return HandleRenderingError(result);
    }

    return true;
}

void VulkanRenderer::EnumerateDisplayModes(IP::Vector<DisplayMode>& modes) const
{
    modes.clear();

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (primaryMonitor == nullptr)
    {
        return;
    }

    int modeCount = 0;
    const GLFWvidmode *videoModes = glfwGetVideoModes(primaryMonitor, &modeCount);

    for (int i = 0; i < modeCount; ++i)
    {
        const GLFWvidmode *currentMode = videoModes + i;
        DisplayMode displayMode = {static_cast<uint32_t>(currentMode->width), 
                                   static_cast<uint32_t>(currentMode->height),
                                   static_cast<uint32_t>(currentMode->refreshRate)};
        modes.push_back(displayMode);
    }
}

} // namespace Render
} // namespace IP