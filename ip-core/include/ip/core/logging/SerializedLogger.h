#pragma once

#include <ip/core/logging/ILogger.h>

#include <mutex>

namespace IP
{
namespace Logging
{

class SerializedLogger : public ILogger
{
    public:

        SerializedLogger(IP::UniquePtr<ILogger>&& logger);
        virtual ~SerializedLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        IP::UniquePtr<ILogger> m_logger;
        std::mutex m_loggerLock;
};

} // namespace Logging
} // namespace IP