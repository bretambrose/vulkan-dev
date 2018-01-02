#include <ip/core/utils/SystemUtils.h>

#include <ip/core/memory/stl/StringStream.h>

#ifdef _WINDOWS
#include <Windows.h>
#endif // _WINDOWS

namespace IP
{
namespace System
{

#ifdef _WINDOWS
IP::String GetProcessId()
{
    uint64_t id = 0;
#if defined(_WINDOWS)
    id = ::GetCurrentProcessId();
#else
    //TODO
#endif

    IP::StringStream ss;
    ss << id;

    return ss.str();
}
#endif // _WINDOWS

IP::String AppendProcessId(const IP::String& value)
{
    return value + "_" + GetProcessId();
}

}
}