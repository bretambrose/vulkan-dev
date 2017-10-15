
#pragma once

#include <type_traits>

namespace IP
{
namespace Enum
{

// make all functions constexpr when compiler supports
template < typename T >
bool Are_All_Enum_Flags_Set( T value, T mask )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	return ( static_cast< BaseEnumType >( value ) & static_cast< BaseEnumType >( mask ) ) == static_cast< BaseEnumType >( mask );
}

template < typename T >
bool Is_An_Enum_Flag_Set( T value, T mask )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	return ( static_cast< BaseEnumType >( value ) & static_cast< BaseEnumType >( mask ) ) != 0;
}

template < typename T >
T Make_Enum_Mask( T flag )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	return flag;
}

template < typename T, typename... Args >
T Make_Enum_Mask( T first, Args... rest )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	BaseEnumType rest_value = static_cast< BaseEnumType >( Make_Enum_Mask( rest... ) );

	return static_cast< T >( static_cast< BaseEnumType >( first ) | rest_value );
}

} // namespace Enum
} // namespace IP
