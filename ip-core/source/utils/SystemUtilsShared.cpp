#include <ip/core/utils/SystemUtils.h>

namespace IP
{
namespace System
{

IP::String AppendProcessId(const IP::String& value)
{
    return value + "_" + GetProcessId();
}

}
}