#include <ip/core/utils/SystemUtils.h>

#include <ip/core/memory/stl/StringStream.h>

#include <Windows.h>

namespace IP
{
namespace System
{

IP::String GetProcessId()
{
    uint64_t id = 0;
    id = ::GetCurrentProcessId();

    IP::StringStream ss;
    ss << id;

    return ss.str();
}


}
}