#include <ip/render/utilities/BackgroundRenderingThread.h>

#include <ip/render/IPRender.h>
#include <ip/render/IRenderer.h>
#include <ip/render/RendererApiType.h>
#include <ip/render/RendererConfig.h>

namespace IP
{
namespace Render
{

static void BackgroundRenderingThreadFunction(const IP::Render::RendererConfig& config)
{
    auto renderer = IP::Render::BuildRenderer(IP::Render::RendererApiType::Vulkan);
    if (!renderer)
    {
        return;
    }

    renderer->Initialize(config);
    renderer->Run();
}

BackgroundRenderingThread::BackgroundRenderingThread() :
    m_backgroundThread(nullptr)
{
}

BackgroundRenderingThread::~BackgroundRenderingThread()
{
    Join();
}

void BackgroundRenderingThread::Start(const IP::Render::RendererConfig& config)
{
    m_backgroundThread = IP::MakeUnique<std::thread>(MEMORY_TAG, BackgroundRenderingThreadFunction, config);
}

void BackgroundRenderingThread::Join()
{
    if (m_backgroundThread)
    {
        m_backgroundThread->join();
        m_backgroundThread = nullptr;
    }
}


} // namespace Render
} // namespace IP