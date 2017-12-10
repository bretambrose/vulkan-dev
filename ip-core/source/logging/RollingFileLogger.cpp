#include <ip/core/logging/RollingFileLogger.h>

namespace IP
{
namespace Logging
{

RollingFileLogger::RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter) :
    m_formatter(std::move(formatter))
{
}

RollingFileLogger::~RollingFileLogger()
{
    m_formatter = nullptr;
}

void RollingFileLogger::Log(LogEntry&& entry)
{
    IP_UNREFERENCED_PARAM(entry);
}

} // namespace Logging
} // namespace IP
