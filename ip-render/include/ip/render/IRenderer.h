#pragma once

#include <ip/core/memory/stl/Vector.h>

namespace IP
{

struct DisplayMode;
struct RendererConfig;

class IRenderer
{
    public:

        virtual ~IRenderer() {}

        virtual void Initialize(const RendererConfig& config) = 0;
        virtual bool HandleInput() = 0;
        virtual void RenderFrame() = 0;
        virtual void Shutdown() = 0;

        virtual const RendererConfig& GetConfig() const = 0;

        virtual void EnumerateDisplayModes(IP::Vector<IP::DisplayMode>& modes) const = 0;
       
};

} // namespace IP


