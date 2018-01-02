#include <ip/core/logging/RollingFileLogger.h>

#include <ip/core/utils/FileUtils.h>
#include <ip/core/utils/SystemUtils.h>
#include <fstream>

namespace IP
{
namespace Logging
{

static const char* LOG_ARCHIVE_DIRECTORY = "archive";
static const char* LOG_FILE_SUFFIX = ".log";

RollingFileLogger::RollingFileLogger(IP::UniquePtr<ILogLineFormatter>&& formatter, IP::String filenamePrefix, IP::String directory) :
    m_formatter(std::move(formatter)),
    m_outputStream(nullptr),
    m_outputLogInterval(),
    m_filenamePrefix(filenamePrefix),
    m_directory(directory)
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

IP::String RollingFileLogger::BuildLogFileName(IP::Time::SystemTimePoint logInterval) const
{
    IP::StringStream filename;
    filename << m_filenamePrefix << "_";
    filename << IP::System::GetProcessId() << "_";

    filename << IP::Time::ConvertSystemTimeToFileSuffix(logInterval);
    filename << LOG_FILE_SUFFIX;

    return IP::FileUtils::JoinPath(m_directory, filename.str());
}

void RollingFileLogger::InitializeAndCleanDirectories(void)
{
    // Create base directory if necessary
    if (!FileUtils::DirectoryExists(m_directory))
    {
        FileUtils::CreateDirectoryByPath(m_directory);
    }

    // Create archive directory if necessary
    IP::Vector<IP::String> directoryComponents;
    IP::FileUtils::SplitPath(m_directory, directoryComponents);

    directoryComponents.push_back(LOG_ARCHIVE_DIRECTORY);

    IP::String archivePath = IP::FileUtils::JoinPath(directoryComponents);
    if (!FileUtils::DirectoryExists(archivePath))
    {
        FileUtils::CreateDirectoryByPath(archivePath);
    }

    // Enumerate all files matching the prefix
    IP::StringStream movePatternString;
    movePatternString << m_filenamePrefix << "*" << LOG_FILE_SUFFIX;

    IP::Vector<IP::String> logFilesToMove;
    IP::FileUtils::EnumerateMatchingFiles(IP::FileUtils::JoinPath(m_directory, movePatternString.str()), logFilesToMove);

    // Attempt to move all matching files to archive directory
    for (const auto& oldFilePath : logFilesToMove)
    {
        IP::String oldFileName = IP::FileUtils::ExtractFilename(oldFilePath);
        IP::String newFilePath = IP::FileUtils::JoinPath(archivePath, oldFileName);
        IP::FileUtils::MoveFileByName(oldFilePath, newFilePath);
    }

    // Enumerate all archive files
    IP::Vector<IP::String> archivedLogs;
    IP::FileUtils::EnumerateMatchingFiles(IP::FileUtils::JoinPath(archivePath, "*"), archivedLogs);

    // Delete files older than a certain time period
    auto currentTime = IP::Time::GetCurrentSystemTime();
    for (const auto& archivedLogFilePath : archivedLogs)
    {
        IP::Time::SystemTimePoint lastModifiedTime;
        bool isValid = IP::Time::GetFileLastModifiedTime(archivedLogFilePath, lastModifiedTime);
        if (!isValid)
        {
            continue;
        }

        auto time_difference = currentTime - lastModifiedTime;

        if (std::chrono::duration_cast< std::chrono::seconds >(time_difference) > std::chrono::duration_cast< std::chrono::seconds >(std::chrono::hours(24)))
        {
            IP::FileUtils::DeleteFileByName(archivedLogFilePath);
        }
    }
}

} // namespace Logging
} // namespace IP
