#pragma once

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/Vector.h>

#include <GLFW/glfw3.h>

namespace IP
{

class IPException;

struct RenderConfig
{
    IP::String m_windowName;

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;
};

class IPRender
{
    public:

        IPRender();
        ~IPRender();

        void Initialize(const RenderConfig& config);
        bool HandleInput();
        void Shutdown();

    private:

        void InitializeVulkan();
        void CleanupVulkan();

        void InitializeVulkanInstance();

        void InitializeValidationCallback();
        void CleanupValidationCallback();

        void BuildVulkanExtensionSet();
        IP::Vector<IP::String> GetOptionalVulkanExtensions() const;

        void BuildValidationLayerSet();
        IP::Vector<IP::String> GetOptionalValidationLayers() const;

        bool IsValidRenderConfig(const RenderConfig& config) const;

        RenderConfig m_config;

        GLFWwindow *m_window;

        VkInstance m_vulkanInstance;
        VkDebugReportCallbackEXT m_validationCallback;

        IP::Vector<IP::String> m_extensionNames;
        IP::Vector<IP::String> m_validationLayerNames;

        bool m_glfwTerminate;
       
};

} // namespace IP


