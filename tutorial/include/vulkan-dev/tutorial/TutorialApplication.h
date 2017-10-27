#pragma once

#include <ip/core/memory/Memory.h>

namespace IP
{
    class IRenderer;
}

class TutorialApplication 
{
    public:

        TutorialApplication();
        ~TutorialApplication();

        void Initialize();

        void Run();

        void Shutdown();

    private:

        IP::UniquePtr<IP::IRenderer> m_renderingSystem;

};
