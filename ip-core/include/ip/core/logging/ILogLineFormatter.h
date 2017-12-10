#pragma once

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/StringStream.h>

namespace IP
{
namespace Logging
{

struct LogEntry;

class ILogLineFormatter
{
    public:

        virtual ~ILogLineFormatter() {}

        virtual void FormatLogLine(IP::StringStream& ss, LogEntry&& entry) const = 0;
};

}
}
