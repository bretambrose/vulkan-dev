
#pragma once

#include <ip/core/memory/Memory.h>

#include <map>

namespace IP
{

template< typename K, 
          typename V, 
          typename P = std::less< K > >
using Map = std::map< K, V, P, IP::Allocator< std::pair< const K, V > > >;

template< typename K, 
          typename V, 
          typename P = std::less< K > >
using MultiMap = std::multimap< K, V, P, IP::Allocator< std::pair< const K, V > > >;

} // namespace IP
