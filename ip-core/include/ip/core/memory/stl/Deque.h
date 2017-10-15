
#pragma once

#include <ip/core/memory/Memory.h>

#include <deque>

namespace IP
{

template< typename T >
using Deque = std::deque< T, IP::Allocator< T > >;

} // namespace IP
