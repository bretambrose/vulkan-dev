#pragma once

#include <ip/core/memory/stl/String.h>
#include <ip/core/utils/TimeUtils.h>

namespace IP
{
namespace Logging
{

struct LogEntry
{
    LogEntry();
    LogEntry(const char* levelName, IP::String&& text, IP::Time::SystemTimePoint time);
    LogEntry(const LogEntry& entry);
    LogEntry(LogEntry&& entry);

    LogEntry& operator =(const LogEntry& entry);
    LogEntry& operator =(LogEntry&& entry);

    const char* m_levelName;
    IP::String m_text;
    IP::Time::SystemTimePoint m_time;
};

}
}
