
#pragma once

#include <ip/core/memory/Memory.h>

#include <unordered_map>

namespace IP
{

template< typename K, 
          typename V, 
          typename H = std::hash< K >, 
          typename E = std::equal_to< K > >
using UnorderedMap = std::unordered_map< K, V, H, E, IP::Allocator< std::pair< const K, V > > >;

template< typename K, 
          typename V, 
          typename H = std::hash< K >, 
          typename E = std::equal_to< K > >
using UnorderedMultiMap = std::unordered_multimap< K, V, H, E, IP::Allocator< std::pair< const K, V > > >;

} // namespace IP
