#include <ip/core/logging/SerializedLogger.h>

namespace IP
{
namespace Logging
{

SerializedLogger::SerializedLogger(IP::UniquePtr<ILogger>&& logger) :
    m_logger(std::move(logger))
{
}

SerializedLogger::~SerializedLogger()
{
    m_logger = nullptr;
}

void SerializedLogger::Log(LogEntry&& entry)
{
    std::lock_guard<std::mutex> lock(m_loggerLock);

    m_logger->Log(std::move(entry));
}

} // namespace Logging
} // namespace IP