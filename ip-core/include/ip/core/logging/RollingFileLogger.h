#pragma once

#include <ip/core/logging/ILogger.h>

#include <ip/core/logging/ILogLineFormatter.h>

namespace IP
{
namespace Logging
{

class RollingFileLogger : public ILogger
{
    public:
        RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter);
        virtual ~RollingFileLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        IP::UniquePtr<ILogLineFormatter> m_formatter;
};

} // namespace Logging
} // namespace IP
