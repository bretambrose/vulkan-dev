#pragma once

#include <ip/core/memory/stl/String.h>

namespace IP
{

enum class RendererApiType;

class IRenderer;
struct RendererConfig;

class IPRender
{
    public:

        static IP::UniquePtr<IRenderer> BuildRenderer(RendererApiType api);

       
};

} // namespace IP


