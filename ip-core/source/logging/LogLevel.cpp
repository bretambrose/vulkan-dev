#include <ip/core/logging/LogLevel.h>

namespace IP
{
namespace Logging
{

static const char* s_logLevelNames[] = {
    "Trace",
    "Debug",
    "Info",
    "Warn",
    "Error",
    "Fatal",
    "None"
};

const char* GetLogLevelName(LogLevel level)
{
    return s_logLevelNames[static_cast<int>(level)];
}

} // namespace Logging
} // namespace IP