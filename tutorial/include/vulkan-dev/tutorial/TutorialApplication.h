#pragma once

#include <ip/core/memory/Memory.h>

namespace IP
{
namespace Render
{

    class BackgroundRenderingThread;

} // namespace Render
} // namespace IP

class TutorialApplication 
{
    public:

        TutorialApplication();
        ~TutorialApplication();

        void Start();
        void Stop();

    private:

        IP::UniquePtr<IP::Render::BackgroundRenderingThread> m_renderThread;

};
