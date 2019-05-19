#include <ip/core/utils/SystemUtils.h>

#include <ip/core/memory/stl/StringStream.h>

#include <sys/types.h>
#include <unistd.h>

namespace IP
{
namespace System
{

IP::String GetProcessId()
{
    pid_t id = 0;
    id = getpid();

    IP::StringStream ss;
    ss << id;

    return ss.str();
}

}
}