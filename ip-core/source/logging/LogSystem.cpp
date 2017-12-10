
#include <ip/core/logging/LogSystem.h>

#include <atomic>

#include <ip/core/logging/ILogger.h>
#include <ip/core/logging/LogLevel.h>

namespace IP
{
namespace Logging
{

static std::atomic<LogLevel> s_logLevel(LogLevel::Debug);
static std::atomic<ILogger*> s_logger(nullptr);

void Initialize(ILogger* logger)
{
    s_logger = logger;
}

void Shutdown()
{
    s_logger = nullptr;
}

void Log(LogEntry&& entry)
{
    if (s_logger != nullptr)
    {
        s_logger.load()->Log(std::move(entry));
    }
}

LogLevel GetLogLevel()
{
    return s_logLevel.load();
}

void SetLogLevel(LogLevel level)
{
    s_logLevel.store(level);
}

} // namespace Logging
} // namespace IP