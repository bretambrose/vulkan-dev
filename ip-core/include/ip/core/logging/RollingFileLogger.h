#pragma once

#include <ip/core/logging/ILogger.h>

#include <ip/core/logging/ILogLineFormatter.h>
#include <ip/core/memory/stl/Stream.h>
#include <ip/core/memory/stl/UnorderedMap.h>
#include <ip/core/utils/TimeUtils.h>

#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace IP
{
namespace Logging
{

class RollingFileLogger : public ILogger
{
    public:
        RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter, IP::String filenamePrefix, const std::experimental::filesystem::path& directory);
        virtual ~RollingFileLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        void InitializeAndCleanDirectories();

        void RollLogFile(void);
        std::experimental::filesystem::path BuildLogFileName(IP::Time::SystemTimePoint logInterval) const;

        IP::UniquePtr<ILogLineFormatter> m_formatter;

        IP::UniquePtr<IP::OFStream> m_outputStream;
        IP::Time::SystemTimePoint m_outputLogInterval;

        std::experimental::filesystem::path m_logFileDirectory;
        IP::String m_filenamePrefix;
};

} // namespace Logging
} // namespace IP
