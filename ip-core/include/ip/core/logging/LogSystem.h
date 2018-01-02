
#pragma once

#include <ip/core/logging/LogEntry.h>
#include <ip/core/logging/LogLevel.h>
#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/StringStream.h>
#include <ip/core/utils/TimeUtils.h>

namespace IP
{
namespace Logging
{

class ILogger;

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
        IP::Logging::Log(IP::Logging::LogEntry(IP::Logging::GetLogLevelName(level), std::move(ss.str()), IP::Time::GetCurrentSystemTime())); \
    }
    
#define LOG_TRACE(...) LOG(IP::Logging::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(IP::Logging::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(IP::Logging::LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...) LOG(IP::Logging::LogLevel::Warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG(IP::Logging::LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...) LOG(IP::Logging::LogLevel::Fatal, __VA_ARGS__)


