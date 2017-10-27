#pragma once

#include <stdint.h>

namespace IP
{

class VulkanDeviceProperties
{
    public:
        VulkanDeviceProperties() :
            m_graphicsQueueFamilyIndex(-1),
            m_presentationQueueFamilyIndex(-1)
        {}

        int32_t m_graphicsQueueFamilyIndex;
        int32_t m_presentationQueueFamilyIndex;
};

} // namespace IP


