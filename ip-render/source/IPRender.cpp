#include <ip/render/IPRender.h>

#include <ip/core/memory/Memory.h>

#include <ip/render/RendererApiType.h>
#include <ip/render/vulkan/VulkanRenderer.h>

namespace IP
{
namespace Render
{

IP::UniquePtr<IRenderer> BuildRenderer(RendererApiType api)
{
    IP::UniquePtr<IRenderer> renderer = nullptr;

    switch (api)
    {
        case RendererApiType::Vulkan:
            renderer = IP::MakeUniqueUpcast<VulkanRenderer, IRenderer>(MEMORY_TAG);
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

} // namespace Render
} // namespace IP