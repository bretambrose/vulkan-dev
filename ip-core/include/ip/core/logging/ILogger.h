#pragma once

#include <functional>

#include <ip/core/memory/stl/String.h>

namespace IP
{
namespace Logging
{

struct LogEntry
{
    const char* m_levelName;
    IP::String m_text;
};

class ILogger
{
    public:

        virtual ~ILogger() {}

        virtual void Log(LogEntry&& entry) = 0;
};

using LoggerFactory = std::function<IP::UniquePtr<ILogger>()>;

}
}
