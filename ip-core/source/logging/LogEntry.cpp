#include <ip/core/logging/LogEntry.h>

namespace IP
{
namespace Logging
{

LogEntry::LogEntry() :
    m_levelName(""),
    m_text(""),
    m_time()
{
}

LogEntry::LogEntry(const char* levelName, IP::String&& text, IP::Time::SystemTimePoint time) :
    m_levelName(levelName),
    m_text(std::move(text)),
    m_time(time)
{
}

LogEntry::LogEntry(const LogEntry& entry) :
    m_levelName(entry.m_levelName),
    m_text(entry.m_text),
    m_time(entry.m_time)
{
}

LogEntry::LogEntry(LogEntry&& entry) :
    m_levelName(entry.m_levelName),
    m_text(std::move(entry.m_text)),
    m_time(entry.m_time)
{
}

LogEntry& LogEntry::operator =(const LogEntry& entry)
{
    m_levelName = entry.m_levelName;
    m_text = entry.m_text;
    m_time = entry.m_time;

    return *this;
}

LogEntry& LogEntry::operator =(LogEntry&& entry)
{
    m_levelName = entry.m_levelName;
    m_text = std::move(entry.m_text);
    m_time = entry.m_time;

    return *this;
}

}
}