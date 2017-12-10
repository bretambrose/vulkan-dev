#pragma once

#include <thread>

#include <ip/core/logging/ILogger.h>

namespace IP
{
namespace Logging
{

struct BackgroundLoggerThreadData;

class BackgroundLogger : public ILogger
{
    public:

        BackgroundLogger(const LoggerFactory& factory);
        virtual ~BackgroundLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        std::shared_ptr<BackgroundLoggerThreadData> m_threadData;
        IP::UniquePtr<std::thread> m_backgroundThread;

};

} // namespace Logging
} // namespace IP