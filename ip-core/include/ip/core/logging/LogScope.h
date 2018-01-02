#pragma once

#include <ip/core/memory/Memory.h>

#include <ip/core/logging/ILogger.h>

namespace IP
{
namespace Logging
{

class LogScope {
    public:
        
        LogScope(IP::UniquePtr<ILogger> &&logger);
        ~LogScope();

    private:

        IP::UniquePtr<ILogger> m_logger;
};

} // namespace Logging
} // namespace IP
