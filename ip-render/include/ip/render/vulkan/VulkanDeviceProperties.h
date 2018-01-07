#pragma once

#include <stdint.h>

namespace IP
{
namespace Render
{

class VulkanDeviceProperties
{
    public:
        VulkanDeviceProperties() :
            m_graphicsQueueFamilyIndex(-1),
            m_presentationQueueFamilyIndex(-1),
            m_supportsRequiredExtensions(false),
            m_surfaceCapabilities(),
            m_surfaceFormats(),
            m_presentationModes()
        {}

        bool MeetsMinimumRequirements() const { 
            return m_graphicsQueueFamilyIndex >= 0 && m_presentationQueueFamilyIndex >= 0 && m_supportsRequiredExtensions && !m_surfaceFormats.empty() && !m_presentationModes.empty(); 
        }

        bool MeetsOptimumQueueRequirements() const {
            return m_graphicsQueueFamilyIndex >= 0 && m_presentationQueueFamilyIndex >= 0 && m_supportsRequiredExtensions && m_graphicsQueueFamilyIndex == m_presentationQueueFamilyIndex; 
        }

        int32_t m_graphicsQueueFamilyIndex;
        int32_t m_presentationQueueFamilyIndex;
        bool m_supportsRequiredExtensions;

        VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
        std::vector<VkPresentModeKHR> m_presentationModes;
};

} // namespace Render
} // namespace IP


