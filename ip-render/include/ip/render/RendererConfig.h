#pragma once

#include <ip/core/memory/stl/String.h>


namespace IP
{
namespace Render
{

enum class RenderDebugLevel;

struct RendererConfig
{
    RenderDebugLevel m_debugLevel;

    IP::String m_windowName;

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    bool m_windowed;
};

} // namespace Render
} // namespace IP


