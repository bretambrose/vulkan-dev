#include <ip/render/IPRender.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include <ip/core/debug/IPException.h>
#include <ip/core/memory/stl/Set.h>
#include <ip/core/UnreferencedParam.h>
#include <ip/render/GlfwError.h>

#include <iostream>

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
static const std::vector<IP::String> s_debugValidationLayers = {};
#else
static const std::vector<IP::String> s_debugValidationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};
#endif

IPRender::IPRender() :
    m_config(),
    m_window(nullptr),
    m_vulkanInstance(nullptr),
    m_validationCallback(nullptr),
    m_extensionNames(),
    m_validationLayerNames(),
    m_glfwTerminate(false)
{
    
}

IPRender::~IPRender()
{
    Shutdown();
}

void IPRender::Initialize(const RenderConfig& config)
{
    if(!IsValidRenderConfig(config)) 
    {
        throw BuildIPException("Invalid render config");
    }

    m_config = config;

    glfwSetErrorCallback(GlfwErrorTracker::GlfwErrorCallback);

    int initResult = glfwInit();
    if (initResult != GLFW_TRUE)
    {
        throw BuildIPException("Failure initializing glfw.  Error code: ", initResult);
    }

    m_glfwTerminate = true;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_config.m_windowWidth, m_config.m_windowHeight, m_config.m_windowName.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
    {
        throw BuildIPException("Glfw failed to build window");
    }

    InitializeVulkan();
}

void IPRender::Shutdown()
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

bool IPRender::HandleInput()
{
    if (glfwWindowShouldClose(m_window)) 
    {
        return false;
    }

    glfwPollEvents();

    return true;
}

bool IPRender::IsValidRenderConfig(const RenderConfig& config) const
{
    if (config.m_windowHeight <= 0 || config.m_windowWidth <= 0) 
    {
        return false;
    }

    return true;
}

void IPRender::InitializeVulkan()
{
    InitializeVulkanInstance();
    InitializeValidationCallback();
}

void IPRender::InitializeVulkanInstance()
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
        throw BuildIPException("Failed to initialize Vulkan instance, error code: ", (int)result);
    }
}

void IPRender::CleanupVulkan()
{
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

void IPRender::BuildVulkanExtensionSet()
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
            throw BuildIPException("Vulkan drivers missing required extension: ", requiredExtension);
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

IP::Vector<IP::String> IPRender::GetOptionalVulkanExtensions() const
{
    IP::Vector<IP::String> optionalExtensions;

    return optionalExtensions;
}

void IPRender::BuildValidationLayerSet()
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
            throw BuildIPException("Vulkan drivers missing required validation layer: ", requiredLayer);
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

IP::Vector<IP::String> IPRender::GetOptionalValidationLayers() const
{
    IP::Vector<IP::String> optionalLayers;

    return optionalLayers;
}

void IPRender::InitializeValidationCallback()
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
        throw BuildIPException("Could not find required extension function: ", CREATE_DEBUG_CALLBACK_PROCEDURE_NAME);
    }

    VkResult result = createCallbackFunc(m_vulkanInstance, &createInfo, nullptr, &m_validationCallback);
    if (result != VK_SUCCESS)
    {
        throw BuildIPException("Failed to create validation callback with error: ", (int32_t) result);
    }
}

void IPRender::CleanupValidationCallback()
{
    auto destroyFunc = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(m_vulkanInstance, DESTROY_DEBUG_CALLBACK_PROCEDURE_NAME);
    if (destroyFunc != nullptr) 
    {
        destroyFunc(m_vulkanInstance, m_validationCallback, nullptr);
    }
}

} // namespace IP