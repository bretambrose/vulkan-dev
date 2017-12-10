#pragma once

#include <ip/core/memory/stl/String.h>

namespace IP
{

struct RendererConfig
{
    IP::String m_windowName;

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    bool m_windowed;
};

} // namespace IP


