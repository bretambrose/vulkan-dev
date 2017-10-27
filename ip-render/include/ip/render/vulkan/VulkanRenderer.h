#pragma once

#include <GLFW/glfw3.h>

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/Vector.h>

#include <ip/render/IRenderer.h>
#include <ip/render/RendererConfig.h>

namespace IP
{

class IPException;
class VulkanDeviceProperties;

class VulkanRenderer : public IRenderer
{
    public:

        VulkanRenderer();
        virtual ~VulkanRenderer();

        virtual void Initialize(const RendererConfig& config) override;
        virtual bool HandleInput() override;
        virtual void Shutdown() override;

    private:

        void InitializeVulkan();
        void CleanupVulkan();

        void InitializeVulkanInstance();

        void InitializeValidationCallback();
        void CleanupValidationCallback();

        void InitializeSurface();

        void InitializeDevice();
        int32_t ScorePhysicalDevice(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const;

        void BuildVulkanExtensionSet();
        IP::Vector<IP::String> GetOptionalVulkanExtensions() const;

        void BuildValidationLayerSet();
        IP::Vector<IP::String> GetOptionalValidationLayers() const;

        bool IsValidRenderConfig(const RendererConfig& config) const;

        RendererConfig m_config;

        GLFWwindow *m_window;

        VkInstance m_vulkanInstance;
        VkDebugReportCallbackEXT m_validationCallback;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_logicalDevice;
        VkSurfaceKHR m_surface;

        IP::Vector<IP::String> m_extensionNames;
        IP::Vector<IP::String> m_validationLayerNames;

        bool m_glfwTerminate;
       
};

} // namespace IP


