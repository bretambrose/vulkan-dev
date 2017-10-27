
#pragma once

#include <ip/core/UnreferencedParam.h>

#include <memory>

namespace IP
{

void *Malloc( const char* tag, size_t memory_size );
void Free( void *memory_ptr );

template< typename T, typename ...Args >
T *New( const char *tag, Args&&... args )
{
	void *raw_memory = IP::Malloc( tag, sizeof( T ) );
	if ( raw_memory == nullptr )
	{
		return nullptr;
	}

	bool should_construct = std::is_class<T>::value;
	if ( should_construct )
	{
		new (raw_memory) T( std::forward< Args >( args )... );
	}

	return static_cast< T * >( raw_memory );
}

template< typename T >
T *NewArray( const char* tag, size_t count )
{
	if ( count <= 0 )
	{
		return nullptr;
	}

	size_t total_allocation = sizeof( T ) * count + sizeof( size_t );

	void *raw_memory = IP::Malloc( tag, total_allocation );
	if ( raw_memory == nullptr )
	{
		return nullptr;
	}

	size_t *count_memory = static_cast< size_t * >( raw_memory );
	*count_memory = count;

	T *t_memory = reinterpret_cast< T * >( count_memory + 1 );
	bool should_construct = std::is_class<T>::value;
	if ( should_construct )
	{
		for ( uint32_t i = 0; i < count; ++i )
		{
			new ( t_memory + i ) T();
		}
	}

	return t_memory;
}

template< typename T >
void Delete( T *object )
{
	if ( object == nullptr )
	{
		return;
	}

	bool should_destruct = !std::is_trivially_destructible<T>::value;
	if ( should_destruct )
	{
		object->~T();
	}

	IP::Free( static_cast< void * >( object ) );
}

template< typename T >
void Delete( const T *object )
{
	Delete( const_cast< T * >( object ) );
}

template< typename T >
void DeleteArray( T *object_array )
{
	if ( object_array == nullptr )
	{
		return;
	}

	size_t *count_memory = reinterpret_cast< size_t * >( reinterpret_cast< void * >( object_array ) ) - 1;

	bool should_destruct = !std::is_trivially_destructible<T>::value;
	if ( should_destruct )
	{
		size_t count = *count_memory;

		for ( uint32_t i = 0; i < count; ++i )
		{
			( object_array + i )->~T();
		}
	}

	IP::Free( static_cast< void * >( count_memory ) );
}

template< typename T >
void DeleteArray( const T *object )
{
	DeleteArray( const_cast< T * >( object ) );
}

/////////////////////////////////////////////////////////////////////

template < typename T >
struct Allocator : public std::allocator< T >
{
	public:

		using Base = std::allocator< T >;

		Allocator() throw() : 
			Base()
		{}

		Allocator( const Allocator< T > &rhs) throw() :
			Base( rhs )
		{}

		template < typename U >                    
		Allocator( const Allocator< U > &rhs ) throw() :
			Base( rhs ) 
		{}

		~Allocator() throw() {}

		typedef std::size_t size_type;

		template< typename U >
		struct rebind
		{
			typedef Allocator< U > other;
		};

		typename Base::pointer allocate( size_type n, const void *hint = nullptr )
		{
			IP_UNREFERENCED_PARAM( hint );

			return reinterpret_cast< typename Base::pointer >( IP::Malloc( "IP::Allocator", n * sizeof( T ) ) );
		}

		void deallocate( typename Base::pointer p, size_type n )
		{
			IP_UNREFERENCED_PARAM( n );

			IP::Free( p );
		}
};

template < typename T >
struct Deleter
{
	public:

		Deleter( bool is_array = false ) :
			IsArray( is_array )
		{}

		template< typename U, class = typename std::enable_if< std::is_convertible< U *, T * >::value, void >::type >
		Deleter( const Deleter< U >& rhs ) :
			IsArray( rhs.GetIsArray() )
		{}

		void operator()(T *object)
		{
			if ( IsArray )
			{
				IP::DeleteArray( object );
			}
			else
			{
				IP::Delete( object );
			}
		}

		bool GetIsArray() const { return IsArray; }

	private:

		bool IsArray;
};

// unique pointers
template< typename T >
using UniquePtr = std::unique_ptr< T, IP::Deleter< T > >;

template< typename T, typename ...Args >
UniquePtr< T > MakeUnique(const char *tag, Args&&... args)
{
	return UniquePtr< T >( IP::New< T >( tag, std::forward< Args >( args )... ), IP::Deleter< T >( false ) );
}

template< typename T >
UniquePtr< T > MakeUniqueArray(const char *tag, size_t array_size)
{
	return UniquePtr< T >( IP::NewArray< T >( tag, array_size ), IP::Deleter< T >( true ) );
}

template< typename T, typename U, typename ...Args >
UniquePtr< U > MakeUniqueUpcast(const char *tag, Args&&... args)
{
	return UniquePtr< U >( IP::New< T >( tag, std::forward< Args >( args )... ), IP::Deleter< U >( false ) );
}

// shared pointers
template< typename T, typename ...Args >
std::shared_ptr< T > MakeShared(const char *tag, Args&&... args)
{
	IP_UNREFERENCED_PARAM( tag );

	return std::allocate_shared< T >( IP::Allocator< T >(), std::forward< Args >( args )... );
}

template< typename T, typename U, typename ...Args >
std::shared_ptr< U > MakeSharedUpcast(const char *tag, Args&&... args)
{
	IP_UNREFERENCED_PARAM( tag );

	return std::static_pointer_cast< U >( std::allocate_shared< T >( IP::Allocator< T >(), std::forward< Args >( args )... ) );
}

} // namespace IP