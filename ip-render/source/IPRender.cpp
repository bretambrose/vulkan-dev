#include <ip/render/IPRender.h>

#include <ip/core/memory/Memory.h>

#include <ip/render/vulkan/VulkanRenderer.h>

static const char* IP_RENDER_ALLOCATION_TAG = "IPRender";

namespace IP
{

IP::UniquePtr<IRenderer> IPRender::BuildRenderer(const RendererConfig& config)
{
    IP::UniquePtr<IRenderer> renderer = IP::MakeUniqueUpcast<IP::VulkanRenderer, IP::IRenderer>(IP_RENDER_ALLOCATION_TAG);
    renderer->Initialize(config);

    return renderer;
}


} // namespace IP