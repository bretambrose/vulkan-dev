#include <ip/core/logging/ConsoleLogger.h>

#include <iostream>

#include <ip/core/logging/ILogLineFormatter.h>
#include <ip/core/memory/stl/StringStream.h>

namespace IP
{
namespace Logging
{

ConsoleLogger::ConsoleLogger(IP::UniquePtr<ILogLineFormatter>&& formatter) :
    m_formatter(std::move(formatter))
{
}

ConsoleLogger::~ConsoleLogger()
{
    m_formatter = nullptr;
}

void ConsoleLogger::Log(LogEntry&& entry)
{
    IP::StringStream ss;
    m_formatter->FormatLogLine(ss, std::move(entry));

    std::cout << ss.str() << std::endl;
}

} // namespace Logging
} // namespace IP

