#include <ip/core/logging/CompositeLogger.h>

namespace IP
{
namespace Logging
{

CompositeLogger::CompositeLogger(IP::Vector<IP::UniquePtr<ILogger>>&& loggers) :
    m_loggers(std::move(loggers))
{
}

CompositeLogger::~CompositeLogger()
{
    m_loggers.clear();
}

void CompositeLogger::Log(LogEntry&& entry)
{
    for (const auto& logger : m_loggers)
    {
        LogEntry entryCopy = entry;
        logger->Log(std::move(entryCopy));
    }
}

} // namespace Logging
} // namespace IP