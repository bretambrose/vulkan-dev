#pragma once

#include <functional>

#include <ip/core/memory/Memory.h>

namespace IP
{
namespace Logging
{

struct LogEntry;

class ILogger
{
    public:

        virtual ~ILogger() {}

        virtual void Log(LogEntry&& entry) = 0;
};

using LoggerFactory = std::function<IP::UniquePtr<ILogger>()>;

}
}
