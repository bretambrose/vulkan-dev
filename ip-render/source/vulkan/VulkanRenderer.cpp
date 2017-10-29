#include <ip/render/vulkan/VulkanRenderer.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>

#include <ip/core/debug/IPException.h>
#include <ip/core/memory/stl/Set.h>
#include <ip/core/UnreferencedParam.h>

#include <ip/render/GlfwError.h>
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

    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

namespace IP
{

static const char* CREATE_DEBUG_CALLBACK_PROCEDURE_NAME = "vkCreateDebugReportCallbackEXT";
static const char* DESTROY_DEBUG_CALLBACK_PROCEDURE_NAME = "vkDestroyDebugReportCallbackEXT";

#ifdef NDEBUG
static const std::vector<const char *> s_debugValidationLayers = {};
#else
static const std::vector<const char *> s_debugValidationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};
#endif

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
    m_vulkanExtensionNames(),
    m_deviceExtensionNames(),
    m_validationLayerNames(),
    m_selectedDeviceProperties(),
    m_swapSurfaceFormat(),
    m_swapPresentationMode(),
    m_swapExtents(),
    m_glfwTerminate(false)
{
    
}

VulkanRenderer::~VulkanRenderer()
{
    Shutdown();
}

void VulkanRenderer::Initialize(const RendererConfig& config)
{
    if(!IsValidRenderConfig(config)) 
    {
       THROW_IP_EXCEPTION("Invalid render config");
    }

    m_config = config;

    glfwSetErrorCallback(GlfwErrorTracker::GlfwErrorCallback);

    int initResult = glfwInit();
    if (initResult != GLFW_TRUE)
    {
        THROW_IP_EXCEPTION("Failure initializing glfw.  Error code: ", initResult);
    }

    m_glfwTerminate = true;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_config.m_windowWidth, m_config.m_windowHeight, m_config.m_windowName.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
    {
        THROW_IP_EXCEPTION("Glfw failed to build window");
    }

    InitializeVulkan();
}

void VulkanRenderer::Shutdown()
{
    CleanupVulkan();

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

bool VulkanRenderer::IsValidRenderConfig(const RendererConfig& config) const
{
    if (config.m_windowHeight <= 0 || config.m_windowWidth <= 0) 
    {
        return false;
    }

    return true;
}

void VulkanRenderer::InitializeVulkan()
{
    InitializeVulkanInstance();
    InitializeValidationCallback();
    InitializeSurface();
    InitializeDevice();
    InitializeSwapChain();
    InitializeSwapChainImageViews();
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

void VulkanRenderer::CleanupVulkan()
{
    CleanupSwapChainImageViews();

    if (m_swapChain)
    {
        vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
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

#ifndef NDEBUG
    requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif // NDEBUG

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
    std::copy(s_debugValidationLayers.cbegin(), s_debugValidationLayers.cend(), std::back_inserter(requiredLayers));

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

    createInfo.enabledLayerCount = static_cast<uint32_t>(s_debugValidationLayers.size());
    if (createInfo.enabledLayerCount > 0)
    {
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

    ExtractPhysicalDeviceProperties(m_physicalDevice, m_selectedDeviceProperties);
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

    VkExtent2D actualExtent = {m_config.m_windowWidth, m_config.m_windowHeight};

    actualExtent.width = std::max(m_selectedDeviceProperties.m_surfaceCapabilities.minImageExtent.width, std::min(m_selectedDeviceProperties.m_surfaceCapabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(m_selectedDeviceProperties.m_surfaceCapabilities.minImageExtent.height, std::min(m_selectedDeviceProperties.m_surfaceCapabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void VulkanRenderer::InitializeSwapChain()
{
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

} // namespace IP