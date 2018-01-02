#include <ip/core/logging/BackgroundLogger.h>

#include <atomic>
#include <condition_variable>

#include <ip/core/logging/LogEntry.h>
#include <ip/core/memory/stl/Vector.h>

namespace IP
{
namespace Logging
{

struct BackgroundLoggerThreadData 
{
    public:

        BackgroundLoggerThreadData(IP::UniquePtr<ILogger>&& logger) :
            m_queueLock(),
            m_entries(),
            m_queueSignal(),
            m_shutdown(false),
            m_backgroundLogger(std::move(logger))
        {}

        ~BackgroundLoggerThreadData() {}

        BackgroundLoggerThreadData(const BackgroundLoggerThreadData& rhs) = delete;
        BackgroundLoggerThreadData(BackgroundLoggerThreadData&& rhs) = delete;
        BackgroundLoggerThreadData& operator =(const BackgroundLoggerThreadData& rhs) = delete;
        BackgroundLoggerThreadData& operator =(BackgroundLoggerThreadData&& rhs) = delete;

        std::mutex m_queueLock;
        IP::Vector<LogEntry> m_entries;
        std::condition_variable m_queueSignal;
        std::atomic<bool> m_shutdown;
        IP::UniquePtr<ILogger> m_backgroundLogger;
};

static void BackgroundThreadFunction(const std::shared_ptr<BackgroundLoggerThreadData>& data)
{
    std::shared_ptr<BackgroundLoggerThreadData> threadData = data;
    bool done = false;

    IP::Vector<LogEntry> entries;

    while (!done)
    {
        entries.clear();

        {
            std::unique_lock<std::mutex> lock(threadData->m_queueLock);
            threadData->m_queueSignal.wait(lock, [&](){return !threadData->m_entries.empty() || threadData->m_shutdown;});

            entries = std::move(threadData->m_entries);
            done = threadData->m_shutdown;
        }

        for (auto& entry : entries)
        {
            threadData->m_backgroundLogger->Log(std::move(entry));
        }

        if (done)
        {
            threadData->m_backgroundLogger = nullptr;
        }
    }
}

BackgroundLogger::BackgroundLogger(const LoggerFactory& factory) :
    m_threadData(IP::MakeShared<BackgroundLoggerThreadData>(MEMORY_TAG, std::move(factory()))),
    m_backgroundThread(IP::MakeUnique<std::thread>(MEMORY_TAG, BackgroundThreadFunction, m_threadData))
{
}

BackgroundLogger::~BackgroundLogger()
{
    if (m_threadData)
    {
        m_threadData->m_shutdown = true;
        m_threadData->m_queueSignal.notify_one();
        m_threadData = nullptr;
    }

    if (m_backgroundThread)
    {
        m_backgroundThread->join();
    }
}

void BackgroundLogger::Log(LogEntry&& entry)
{
    if (m_threadData)
    {
        {
            std::unique_lock<std::mutex> queueLock(m_threadData->m_queueLock);
            m_threadData->m_entries.push_back(std::move(entry));
        }

        m_threadData->m_queueSignal.notify_one();
    }
}

} // namespace Logging
} // namespace IP