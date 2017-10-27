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
    m_extensionNames(),
    m_validationLayerNames(),
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
}

void VulkanRenderer::InitializeVulkanInstance()
{
    BuildValidationLayerSet();
    IP::Vector<const char *> rawLayerNames;
    std::for_each(m_validationLayerNames.cbegin(), m_validationLayerNames.cend(), [&](const IP::String& name){ rawLayerNames.push_back(name.c_str()); });

    BuildVulkanExtensionSet();
    IP::Vector<const char *> rawExtensionNames;
    std::for_each(m_extensionNames.cbegin(), m_extensionNames.cend(), [&](const IP::String& name){ rawExtensionNames.push_back(name.c_str()); });

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
    if (m_logicalDevice)
    {
        vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = nullptr;
    }

    if (m_surface)
    {
        vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
        m_surface = nullptr;
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

        m_extensionNames.push_back(requiredExtension);
    }

    // build and check optional extensions, filter out on missing
    IP::Vector<IP::String> optionalExtensions = GetOptionalVulkanExtensions();
    for (const auto& extensionName : optionalExtensions)
    {
        if (presentExtensions.find(extensionName) != presentExtensions.cend())
        {
            m_extensionNames.push_back(extensionName);
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

    createInfo.enabledExtensionCount = 0;

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
}

int32_t VulkanRenderer::ScorePhysicalDevice(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const
{
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

        // if we've found a single queue that satisfies all of our needs then stop, otherwise keep looking
        if (deviceProperties.m_graphicsQueueFamilyIndex >= 0 && deviceProperties.m_graphicsQueueFamilyIndex == deviceProperties.m_presentationQueueFamilyIndex)
        {
            break;
        }
    }

    if (deviceProperties.m_graphicsQueueFamilyIndex >= 0 && deviceProperties.m_presentationQueueFamilyIndex >= 0)
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

} // namespace IP