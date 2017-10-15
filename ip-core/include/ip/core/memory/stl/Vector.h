
#pragma once

#include <ip/core/memory/Memory.h>

#include <vector>

namespace IP
{

template< typename T >
using Vector = std::vector< T, IP::Allocator< T > >;

} // namespace IP
