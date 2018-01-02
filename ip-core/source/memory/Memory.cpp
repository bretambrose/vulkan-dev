/**********************************************************************************************************************

	(c) Copyright 2011, Bret Ambrose (mailto:bretambrose@gmail.com).

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************************************/

#include <ip/core/memory/Memory.h>

#include <ip/core/UnreferencedParam.h>

namespace IP
{

static IMemoryAllocator* s_globalAllocator = nullptr;

ScopedMemoryAllocator::ScopedMemoryAllocator(IMemoryAllocator* allocator)
{
    s_globalAllocator = allocator;
}

ScopedMemoryAllocator::~ScopedMemoryAllocator()
{
    s_globalAllocator = nullptr;
}

void *Malloc( const char* tag, size_t memory_size )
{
    if (s_globalAllocator)
    {
        return s_globalAllocator->Allocate(tag, memory_size);
    }
    else
    {
        return malloc(memory_size);
    }
}

void Free( void *memory_ptr )
{
    if (s_globalAllocator)
    {
        s_globalAllocator->Free(memory_ptr);
    }
    else
    {
        free(memory_ptr);
    }
}

} // namespace IP