#pragma once

#include <ip/core/memory/stl/Vector.h>
#include <ip/core/memory/stl/String.h>

namespace IP
{
namespace FileUtils
{

    IP::Vector<uint8_t> LoadFileData(const IP::String& fileName);

} // namespace IP
} // namespace IP
