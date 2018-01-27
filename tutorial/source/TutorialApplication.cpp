#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <ip/core/logging/LogSystem.h>

#include <ip/render/utilities/BackgroundRenderingThread.h>

#include <ip/render/DisplayMode.h>
#include <ip/render/IPRender.h>
#include <ip/render/IRenderer.h>
#include <ip/render/RendererApiType.h>
#include <ip/render/RendererConfig.h>
#include <ip/render/RenderDebugLevel.h>

TutorialApplication::TutorialApplication() :
    m_renderThread(nullptr)
{
}

TutorialApplication::~TutorialApplication()
{
}

void TutorialApplication::Start()
{
    LOG_DEBUG("TutorialApplication::Start")

    IP::Render::RendererConfig config = {};
    config.m_debugLevel = IP::Render::RenderDebugLevel::Debug;
    config.m_windowName = "VulkanTutorial";
    config.m_windowWidth = 1024;
    config.m_windowHeight = 768;
    config.m_windowed = true;

    m_renderThread = IP::MakeUnique<IP::Render::BackgroundRenderingThread>(MEMORY_TAG);
    m_renderThread->Start(config);
}

void TutorialApplication::Stop()
{
    LOG_DEBUG("TutorialApplication::Stop")

    if (m_renderThread)
    {
        m_renderThread->Join();
        m_renderThread = nullptr;
    }
}
