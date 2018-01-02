#pragma once

#include <ip/core/memory/stl/String.h>

namespace IP
{
namespace System
{

IP::String GetProcessId();
IP::String AppendProcessId(const IP::String& value);

}
}