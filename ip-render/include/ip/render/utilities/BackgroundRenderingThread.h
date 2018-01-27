#pragma once

#include <ip/core/memory/Memory.h>

#include <thread>

namespace IP
{
namespace Render
{

struct RendererConfig;

class BackgroundRenderingThread
{
    public:

        BackgroundRenderingThread();
        ~BackgroundRenderingThread();

        void Start(const IP::Render::RendererConfig& config);
        void Join();

    private:

        IP::UniquePtr<std::thread> m_backgroundThread;
};

} // namespace Render
} // namespace IP


