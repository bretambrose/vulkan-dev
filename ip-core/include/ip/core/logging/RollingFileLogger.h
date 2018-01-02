#pragma once

#include <ip/core/logging/ILogger.h>

#include <ip/core/logging/ILogLineFormatter.h>
#include <ip/core/memory/stl/Stream.h>
#include <ip/core/memory/stl/UnorderedMap.h>
#include <ip/core/utils/TimeUtils.h>

namespace IP
{
namespace Logging
{

class RollingFileLogger : public ILogger
{
    public:
        RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter, IP::String filenamePrefix = "IP_Process_", IP::String directory = ".");
        virtual ~RollingFileLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        void InitializeAndCleanDirectories();

        void RollLogFile(void);
        IP::String BuildLogFileName(IP::Time::SystemTimePoint logInterval) const;

        IP::UniquePtr<ILogLineFormatter> m_formatter;

        IP::UniquePtr<IP::OFStream> m_outputStream;
        IP::Time::SystemTimePoint m_outputLogInterval;

        IP::String m_directory;
        IP::String m_filenamePrefix;
};

} // namespace Logging
} // namespace IP
