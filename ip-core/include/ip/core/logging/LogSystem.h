
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

#define LOG(level, streamExpression) \
    if (IP::Logging::GetLogLevel() <= level) { \
        IP::StringStream ss; \
        ss << streamExpression; \
        IP::Logging::Log(IP::Logging::LogEntry(IP::Logging::GetLogLevelName(level), std::move(ss.str()), IP::Time::GetCurrentSystemTime())); \
    }
    
#define LOG_TRACE(streamExpression) LOG(IP::Logging::LogLevel::Trace, streamExpression)
#define LOG_DEBUG(streamExpression) LOG(IP::Logging::LogLevel::Debug, streamExpression)
#define LOG_INFO(streamExpression) LOG(IP::Logging::LogLevel::Info, streamExpression)
#define LOG_WARN(streamExpression) LOG(IP::Logging::LogLevel::Warn, streamExpression)
#define LOG_ERROR(streamExpression) LOG(IP::Logging::LogLevel::Error, streamExpression)
#define LOG_FATAL(streamExpression) LOG(IP::Logging::LogLevel::Fatal, streamExpression)


