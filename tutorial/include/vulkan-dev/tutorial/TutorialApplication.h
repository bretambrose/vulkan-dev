#pragma once

#include <ip/core/memory/Memory.h>

namespace IP
{
namespace Render
{

    class IRenderer;

} // namespace Render
} // namespace IP

class TutorialApplication 
{
    public:

        TutorialApplication();
        ~TutorialApplication();

        void Initialize();

        void Run();

        void Shutdown();

    private:

        IP::UniquePtr<IP::Render::IRenderer> m_renderingSystem;

};
