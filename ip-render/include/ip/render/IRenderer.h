#pragma once

namespace IP
{

struct RendererConfig;

class IRenderer
{
    public:

        virtual ~IRenderer() {}

        virtual void Initialize(const RendererConfig& config) = 0;
        virtual bool HandleInput() = 0;
        virtual void Shutdown() = 0;
       
};

} // namespace IP


