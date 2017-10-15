
#pragma once

#include <ip/core/memory/Memory.h>
#include <ip/core/memory/stl/Deque.h>

#include <stack>

namespace IP
{

template< typename T >
using Stack = std::stack< T, IP::Deque< T> >;

} // namespace IP
