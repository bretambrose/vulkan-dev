#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <ip/render/IPRender.h>
#include <ip/render/IRenderer.h>
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
    IP::RendererConfig config = {"VulkanTutorial", 1024, 768};
    m_renderingSystem = IP::IPRender::BuildRenderer(config);
}

void TutorialApplication::Run()
{
    while(m_renderingSystem->HandleInput())
    {
        ;
    }
}

void TutorialApplication::Shutdown()
{
    if(m_renderingSystem)
    {
        m_renderingSystem->Shutdown();
        m_renderingSystem = nullptr;
    }
}
