#pragma once

#include <ip/core/memory/stl/String.h>

namespace IP
{

class IRenderer;
struct RendererConfig;

class IPRender
{
    public:

        static IP::UniquePtr<IRenderer> BuildRenderer(const RendererConfig& config);

       
};

} // namespace IP


