
#pragma once

#include <ip/core/memory/Memory.h>

#include <string>

namespace IP
{

using String = std::basic_string< char, std::char_traits< char >, IP::Allocator< char > >;
using WString = std::basic_string< wchar_t, std::char_traits< wchar_t >, IP::Allocator< wchar_t > >;

} // namespace IP
