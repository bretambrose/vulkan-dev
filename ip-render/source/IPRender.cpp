#include <ip/render/IPRender.h>

#include <ip/core/memory/Memory.h>

#include <ip/render/RendererApiType.h>
#include <ip/render/vulkan/VulkanRenderer.h>

namespace IP
{

IP::UniquePtr<IRenderer> IPRender::BuildRenderer(RendererApiType api)
{
    IP::UniquePtr<IRenderer> renderer = nullptr;

    switch (api)
    {
        case RendererApiType::Vulkan:
            renderer = IP::MakeUniqueUpcast<IP::VulkanRenderer, IP::IRenderer>(MEMORY_TAG);
            break;

        case RendererApiType::OpenGL:
            break;

        case RendererApiType::DirectX:
            break;

        default:
            break;
    }

    return renderer;
}


} // namespace IP