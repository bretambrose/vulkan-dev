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

    IP::Render::RendererConfig config = {IP::Render::RenderDebugLevel::Debug, "VulkanTutorial", 1024, 768, false};
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
