#include <ip/core/logging/RollingFileLogger.h>

#include <ip/core/debug/IPException.h>
#include <ip/core/utils/FileUtils.h>
#include <ip/core/utils/SystemUtils.h>

#include <fstream>

namespace IP
{
namespace Logging
{

static const char* LOG_ARCHIVE_DIRECTORY = "archive";
static const char* LOG_FILE_SUFFIX = ".log";

RollingFileLogger::RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter, IP::String filenamePrefix, const std::experimental::filesystem::path& directory) :
    m_formatter(std::move(formatter)),
    m_outputStream(nullptr),
    m_outputLogInterval(),
    m_filenamePrefix(filenamePrefix),
    m_logFileDirectory(directory)
{
    InitializeAndCleanDirectories();
}

RollingFileLogger::~RollingFileLogger()
{
    if (m_outputStream)
    {
        m_outputStream->close();
    }
}

void RollingFileLogger::Log(LogEntry&& entry)
{
    RollLogFile();

    IP::StringStream ss;
    m_formatter->FormatLogLine(ss, std::move(entry));

    (*m_outputStream) << ss.str() << std::endl;
}

void RollingFileLogger::RollLogFile()
{
    IP::Time::SystemTimePoint currentLogInterval = std::chrono::floor<std::chrono::hours>(IP::Time::GetCurrentSystemTime());
    if (currentLogInterval == m_outputLogInterval)
    {
        return;
    }

    if (m_outputStream)
    {
        m_outputStream->close();
    }

    m_outputStream = IP::MakeUnique<IP::OFStream>(MEMORY_TAG, BuildLogFileName(currentLogInterval).c_str(), std::ios_base::out | std::ios_base::ate);
    m_outputLogInterval = currentLogInterval;
}

std::experimental::filesystem::path RollingFileLogger::BuildLogFileName(IP::Time::SystemTimePoint logInterval) const
{
    IP::StringStream filename;
    filename << m_filenamePrefix << "_";
    filename << IP::System::GetProcessId() << "_";

    filename << IP::Time::ConvertSystemTimeToFileSuffix(logInterval);
    filename << LOG_FILE_SUFFIX;

    std::experimental::filesystem::path fullPath(m_logFileDirectory);
    fullPath.append(filename.str());

    return fullPath;
}

void RollingFileLogger::InitializeAndCleanDirectories(void)
{
    // Create base directory if necessary
    if (!std::experimental::filesystem::is_directory(m_logFileDirectory))
    {
        if (!std::experimental::filesystem::exists(m_logFileDirectory))
        {
            std::experimental::filesystem::create_directory(m_logFileDirectory);
        }
        else
        {
            THROW_IP_EXCEPTION("RollingFileLogger directory exists and is not a directory: ", m_logFileDirectory.c_str());
        }
    }

    // Create archive directory if necessary
    std::experimental::filesystem::path archivePath(m_logFileDirectory);
    archivePath.append(LOG_ARCHIVE_DIRECTORY);

    if (!std::experimental::filesystem::is_directory(archivePath))
    {
        if (!std::experimental::filesystem::exists(archivePath))
        {
            std::experimental::filesystem::create_directory(archivePath);
        }
        else
        {
            THROW_IP_EXCEPTION("RollingFileLogger archive directory exists and is not a directory: ", archivePath.c_str());
        }
    }

    // Enumerate all files matching the prefix
    for (const auto& file : std::experimental::filesystem::directory_iterator(m_logFileDirectory))
    {
        const auto& path = file.path();
        IP::String filename(path.filename().string().c_str());
        if (strncmp(filename.c_str(), m_filenamePrefix.c_str(), m_filenamePrefix.size()))
        {
            continue;
        }

        std::experimental::filesystem::path newPath(archivePath);
        newPath.append(filename);

        std::experimental::filesystem::rename(file, newPath);
    }

    auto currentTime = IP::Time::GetCurrentSystemTime();
    for (const auto& file : std::experimental::filesystem::directory_iterator(archivePath))
    {
        auto lastModifiedTime = std::experimental::filesystem::last_write_time(file);
        auto timeDifference = currentTime - lastModifiedTime;
        if (std::chrono::duration_cast< std::chrono::seconds >(timeDifference) > std::chrono::duration_cast< std::chrono::seconds >(std::chrono::hours(24)))
        {
            std::experimental::filesystem::remove(file);
        }
    }
}

} // namespace Logging
} // namespace IP
