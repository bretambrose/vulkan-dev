
#pragma once

#include <ip/core/memory/stl/String.h>


namespace IP
{
namespace StringUtils
{

	void String_To_WideString( const IP::String &source, IP::WString &target );
	void String_To_WideString( const char *source, IP::WString &target );

	void WideString_To_String( const IP::WString &source, IP::String &target );
	void WideString_To_String( const wchar_t *source, IP::String &target );

	void To_Upper_Case( const IP::String &source, IP::String &dest );
	void To_Upper_Case( const IP::WString &source, IP::WString &dest );

	bool Convert( const IP::String &source, int32_t &value );
	bool Convert( const IP::String &source, uint32_t &value );
	bool Convert( const IP::String &source, int64_t &value );
	bool Convert( const IP::String &source, uint64_t &value );
	bool Convert( const IP::String &source, IP::WString &value );
	bool Convert( const IP::String &source, IP::String &value );
	bool Convert( const IP::String &source, float &value );
	bool Convert( const IP::String &source, double &value );
	bool Convert( const IP::String &source, bool &value );

	bool Convert_Raw( const char *source, int32_t &value );
	bool Convert_Raw( const char *source, uint32_t &value );
	bool Convert_Raw( const char *source, int64_t &value );
	bool Convert_Raw( const char *source, uint64_t &value );
	bool Convert_Raw( const char *source, IP::WString &value );
	bool Convert_Raw( const char *source, IP::String &value );
	bool Convert_Raw( const char *source, float &value );
	bool Convert_Raw( const char *source, double &value );
	bool Convert_Raw( const char *source, bool &value );

} // namespace StringUtils
} // namespace IP

