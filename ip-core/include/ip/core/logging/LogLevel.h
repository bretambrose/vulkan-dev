#pragma once

namespace IP
{
namespace Logging
{

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    None
};

const char* GetLogLevelName(LogLevel level);

} // namespace Logging
} // namespace IP