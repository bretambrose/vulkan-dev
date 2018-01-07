#pragma once

#include <ip/core/memory/stl/String.h>

namespace IP
{
namespace Render
{

enum class RendererApiType;

class IRenderer;
struct RendererConfig;

IP::UniquePtr<IRenderer> BuildRenderer(RendererApiType api);

} // namespace Render
} // namespace IP


