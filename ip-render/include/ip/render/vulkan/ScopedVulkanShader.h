#pragma once

#include <ip/core/memory/stl/Vector.h>

#include <vulkan/vulkan.h>

namespace IP
{ 

class ScopedVulkanShader
{
    public:

        ScopedVulkanShader(const IP::Vector<uint8_t>& data, VkDevice device);
        ~ScopedVulkanShader();

        VkShaderModule GetShaderModule() const { return m_shaderModule; }

    private:

        VkShaderModule m_shaderModule;
        VkDevice m_device;
};

} // namespace IP
