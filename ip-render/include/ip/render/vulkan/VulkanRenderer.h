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
        virtual void RenderFrame() override;
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

        void InitializeSwapChainImageViews();
        void CleanupSwapChainImageViews();

        void InitializeRenderPass();
        void InitializeGraphicsPipeline();

        void InitializeFramebuffers();
        void CleanupFramebuffers();

        void InitializeCommandPool();
        void InitializeCommandBuffers();
        void InitializeSynchronization();

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
        IP::Vector<VkImageView> m_swapChainImageViews;
        VkRenderPass m_renderPass;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;
        IP::Vector<VkFramebuffer> m_swapChainFramebuffers;
        VkCommandPool m_commandPool;
        IP::Vector<VkCommandBuffer> m_commandBuffers;
        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        
        VkQueue m_presentationQueue;
        VkQueue m_graphicsQueue;
        IP::Vector<VkImage> m_swapChainImages;

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


