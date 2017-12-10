#include <ip/core/logging/StandardLogLineFormatter.h>

namespace IP
{
namespace Logging
{

void StandardLogLineFormatter::FormatLogLine(IP::StringStream& ss, LogEntry&& entry) const
{
    IP_UNREFERENCED_PARAM(ss);
    IP_UNREFERENCED_PARAM(entry);
}

} // namespace Logging
} // namespace IP