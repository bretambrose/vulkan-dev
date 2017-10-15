
#pragma once

#include <cstddef>
#include <new>

#ifdef MSVC
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif // MSVC

void *operator new( std::size_t size ) throw( std::bad_alloc );
void *operator new( std::size_t size, const std::nothrow_t &no_throw ) throw();
void *operator new[]( std::size_t size ) throw( std::bad_alloc );
void *operator new[]( std::size_t size, const std::nothrow_t &no_throw ) throw();

void operator delete( void *memory ) throw();
void operator delete( void *memory, const std::nothrow_t &no_throw ) throw();
void operator delete[]( void *memory ) throw();
void operator delete[]( void *memory, const std::nothrow_t &no_throw ) throw();

#ifdef MSVC
#pragma warning( pop ) 
#endif // MSVC

