#include <ip/render/vulkan/ScopedVulkanShader.h>

#include <ip/core/debug/IPException.h>

namespace IP
{
namespace Render
{

ScopedVulkanShader::ScopedVulkanShader(const IP::Vector<uint8_t>& data, VkDevice device) :
    m_shaderModule(nullptr),
    m_device(device)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = data.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule);
    if (result != VK_SUCCESS)
    {
        THROW_IP_EXCEPTION("Failed to create shader module");
    }
}

ScopedVulkanShader::~ScopedVulkanShader()
{
    if (m_shaderModule != nullptr)
    {
        vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
    }
}

} // namespace Render
} // namespace IP
