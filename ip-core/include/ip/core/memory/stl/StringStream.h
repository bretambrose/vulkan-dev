
#pragma once

#include <ip/core/memory/Memory.h>

#include <sstream>

namespace IP
{

typedef std::basic_stringstream< char, std::char_traits< char >, IP::Allocator< char > > StringStream;
typedef std::basic_ostringstream< char, std::char_traits< char >, IP::Allocator< char > > OStringStream;
typedef std::basic_stringbuf< char, std::char_traits< char >, IP::Allocator< char > > StringBuf;

} // namespace IP

