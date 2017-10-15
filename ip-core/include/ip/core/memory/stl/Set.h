
#pragma once

#include <ip/core/memory/Memory.h>

#include <set>

namespace IP
{

template< typename T, 
          typename P = std::less< T > >
using Set = std::set< T, P, IP::Allocator< T > >;


} // namespace IP
