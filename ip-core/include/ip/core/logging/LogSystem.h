
#pragma once

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/StringStream.h>

namespace IP
{
namespace Logging
{

class ILogger;
struct LogEntry;
enum class LogLevel;

// not threadsafe
void Initialize(ILogger* logger);
void Shutdown();

// threadsafe relative to other logging calls, but not Initialize/Shutdown
void Log(LogEntry&& text);

// threadsafe
LogLevel GetLogLevel();
void SetLogLevel(LogLevel level);

} // namespace Logging
} // namespace IP

#define LOG(level, ...) \
    if (IP::Logging::GetLogLevel() <= level) { \
        IP::StringStream ss; \
        ss << __VA_ARGS__; \
        IP::Logging::Log(LogEntry(IP::Logging::GetLogLevelName(level), std::move(ss.str()))); \
    }
    
#define LOG_TRACE(...) LOG(IP::Logging::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(IP::Logging::Debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(IP::Logging::Info, __VA_ARGS__)
#define LOG_WARN(...) LOG(IP::Logging::Warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG(IP::Logging::Error, __VA_ARGS__)
#define LOG_FATAL(...) LOG(IP::Logging::Fatal, __VA_ARGS__)


