#pragma once

#include <memory>

namespace IP
{
    class IPRender;
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

        std::unique_ptr<IP::IPRender> m_renderingSystem;

};
