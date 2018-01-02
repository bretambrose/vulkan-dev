#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <ip/core/logging/LogSystem.h>

#include <ip/render/DisplayMode.h>
#include <ip/render/IPRender.h>
#include <ip/render/IRenderer.h>
#include <ip/render/RendererApiType.h>
#include <ip/render/RendererConfig.h>

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

    IP::RendererConfig config = {"VulkanTutorial", 1024, 768, false};
    m_renderingSystem = IP::IPRender::BuildRenderer(IP::RendererApiType::Vulkan);
    if (!m_renderingSystem)
    {
        return;
    }

    IP::Vector<IP::DisplayMode> modes;
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
        m_renderingSystem->RenderFrame();
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
