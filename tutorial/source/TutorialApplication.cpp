#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <ip/core/logging/LogSystem.h>

#include <ip/render/DisplayMode.h>
#include <ip/render/IPRender.h>
#include <ip/render/IRenderer.h>
#include <ip/render/RendererApiType.h>
#include <ip/render/RendererConfig.h>
#include <ip/render/RenderDebugLevel.h>

TutorialApplication::TutorialApplication() :
    m_renderingSystem(nullptr)
{
}

TutorialApplication::~TutorialApplication()
{
}

void TutorialApplication::Initialize()
{
    LOG_DEBUG("TutorialApplication::Initialize")

    IP::Render::RendererConfig config = {};
    config.m_debugLevel = IP::Render::RenderDebugLevel::Debug;
    config.m_windowName = "VulkanTutorial";
    config.m_windowWidth = 1024;
    config.m_windowHeight = 768;
    config.m_windowed = true;
   
    m_renderingSystem = IP::Render::BuildRenderer(IP::Render::RendererApiType::Vulkan);
    if (!m_renderingSystem)
    {
        return;
    }

    IP::Vector<IP::Render::DisplayMode> modes;
    m_renderingSystem->EnumerateDisplayModes(modes);

    m_renderingSystem->Initialize(config);
}

void TutorialApplication::Run()
{
    if (!m_renderingSystem)
    {
        return;
    }

    while(m_renderingSystem->HandleInput())
    {
        if (!m_renderingSystem->RenderFrame())
        {
            break;
        }
    }
}

void TutorialApplication::Shutdown()
{
    LOG_DEBUG("TutorialApplication::Shutdown")

    if(m_renderingSystem)
    {
        m_renderingSystem->Shutdown();
        m_renderingSystem = nullptr;
    }
}
