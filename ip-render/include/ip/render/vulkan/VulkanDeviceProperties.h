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
            m_name(""),
            m_graphicsQueueFamilyIndex(-1),
            m_presentationQueueFamilyIndex(-1),
            m_supportsRequiredExtensions(false),
            m_surfaceCapabilities(),
            m_surfaceFormats(),
            m_presentationModes(),
            m_extensionNames()
        {}

        bool MeetsMinimumRequirements() const { 
            return m_graphicsQueueFamilyIndex >= 0 && m_presentationQueueFamilyIndex >= 0 && m_supportsRequiredExtensions && !m_surfaceFormats.empty() && !m_presentationModes.empty(); 
        }

        bool MeetsOptimumQueueRequirements() const {
            return m_graphicsQueueFamilyIndex >= 0 && m_presentationQueueFamilyIndex >= 0 && m_supportsRequiredExtensions && m_graphicsQueueFamilyIndex == m_presentationQueueFamilyIndex; 
        }

        IP::String m_name;

        int32_t m_graphicsQueueFamilyIndex;
        int32_t m_presentationQueueFamilyIndex;
        bool m_supportsRequiredExtensions;

        VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
        IP::Vector<VkSurfaceFormatKHR> m_surfaceFormats;
        IP::Vector<VkPresentModeKHR> m_presentationModes;

        IP::Vector<IP::String> m_extensionNames;
};

} // namespace Render
} // namespace IP


