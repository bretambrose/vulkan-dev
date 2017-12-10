#pragma once

#include <ip/core/logging/ILogger.h>

#include <ip/core/logging/ILogLineFormatter.h>

namespace IP
{
namespace Logging
{

class ConsoleLogger : public ILogger
{
    public:
        ConsoleLogger(IP::UniquePtr<ILogLineFormatter>&& formatter);
        virtual ~ConsoleLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        IP::UniquePtr<ILogLineFormatter> m_formatter;
};

} // namespace Logging
} // namespace IP