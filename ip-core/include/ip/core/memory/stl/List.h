
#pragma once

#include <ip/core/memory/Memory.h>

#include <list>

namespace IP
{

template< typename T >
using List = std::list< T, IP::Allocator< T > >;

} // namespace IP
