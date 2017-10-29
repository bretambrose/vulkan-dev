#pragma once

#include <GLFW/glfw3.h>

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/Vector.h>

#include <ip/render/IRenderer.h>
#include <ip/render/RendererConfig.h>
#include <ip/render/vulkan/VulkanDeviceProperties.h>

namespace IP
{

class IPException;

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
        void ExtractPhysicalDeviceProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const;
        void ExtractQueueFamilyProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const;
        void ExtractSwapChainProperties(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const;
        int32_t ScorePhysicalDevice(VkPhysicalDevice device, VulkanDeviceProperties& deviceProperties) const;

        void InitializeSwapChain();
        VkSurfaceFormatKHR SelectSwapSurfaceFormat() const;
        VkPresentModeKHR SelectSwapPresentationMode() const;
        VkExtent2D SelectSwapExtent() const;

        void BuildVulkanExtensionSet();
        IP::Vector<IP::String> GetOptionalVulkanExtensions() const;

        bool BuildDeviceExtensionSet(VkPhysicalDevice device, IP::Vector<IP::String>& extensions) const;
        IP::Vector<IP::String> GetOptionalDeviceExtensions() const;
        IP::Vector<IP::String> GetRequiredDeviceExtensions() const;

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
        VkSwapchainKHR m_swapChain;
        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;

        IP::Vector<IP::String> m_vulkanExtensionNames;
        IP::Vector<IP::String> m_deviceExtensionNames;
        IP::Vector<IP::String> m_validationLayerNames;

        VulkanDeviceProperties m_selectedDeviceProperties;
        VkSurfaceFormatKHR m_swapSurfaceFormat;
        VkPresentModeKHR  m_swapPresentationMode;
        VkExtent2D m_swapExtents;

        bool m_glfwTerminate;
       
};

} // namespace IP


