#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <ip/render/IPRender.h>

TutorialApplication::TutorialApplication() :
    m_renderingSystem(nullptr)
{
}

TutorialApplication::~TutorialApplication()
{
}

void TutorialApplication::Initialize()
{
    m_renderingSystem = std::make_unique<IP::IPRender>();

    IP::RenderConfig config = {"VulkanTutorial", 1024, 768};

    m_renderingSystem->Initialize(config);
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
    m_renderingSystem->Shutdown();
}
