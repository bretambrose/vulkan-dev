#include <ip/core/logging/StandardLogLineFormatter.h>

#include <ip/core/logging/LogEntry.h>

namespace IP
{
namespace Logging
{

void StandardLogLineFormatter::FormatLogLine(IP::StringStream& ss, LogEntry&& entry) const
{
    ss << IP::Time::FormatTimeOfDay(entry.m_time) << " [" << entry.m_levelName << "] " << entry.m_text;
}

} // namespace Logging
} // namespace IP