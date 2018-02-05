#include <ip/core/utils/TimeUtils.h>

#include <ctime>
#include <iomanip>

#include <ip/core/memory/stl/StringStream.h>

namespace IP
{
namespace Time
{

SystemTimePoint GetCurrentSystemTime(void)
{
    return std::chrono::system_clock::now();
}

#ifdef _WINDOWS
bool GetFileLastModifiedTime(const IP::String &fileName, SystemTimePoint& lastModifiedTime)
{
    FILE *fp = nullptr;
    auto openResult = fopen_s(&fp, fileName.c_str(), "r");
    if (fp == nullptr || openResult == 0)
    {
        return false;
    }

    int fd = _fileno(fp);
    if (fd == -1)
    {
        return false;
    }

    struct _stat fileStats;
    auto result = _fstat(fd, &fileStats); // Windows-specific
    if (result != 0)
    {
        return false;
    }

    fclose(fp);

    lastModifiedTime = std::chrono::system_clock::from_time_t(fileStats.st_mtime);
    return true;
}
#endif // WIN32

std::tm localtime(std::time_t time)
{
    std::tm tmSnapshot;
#if defined(_WINDOWS)
    localtime_s(&tmSnapshot, &time);
#else
    localtime_r(&time, &tmSnapshot); // POSIX  
#endif
    return tmSnapshot;
}

IP::String FormatSystemTime(SystemTimePoint timePoint)
{
    auto cTime = std::chrono::system_clock::to_time_t(timePoint);
    auto tmTime = localtime(cTime);
    auto yearRemainder = tmTime.tm_year % 100;

    // Grungy hack assumes that system clock "starts" with 0 milliseconds elapsed
    auto millisecondsElapsed = std::chrono::time_point_cast< std::chrono::milliseconds >(timePoint);
    auto millisecondsRemainder = millisecondsElapsed.time_since_epoch().count() % 1000;

    IP::StringStream ss;
    ss << std::setfill('0') << std::internal;
    ss << std::setw(2) << tmTime.tm_mon + 1 << "-" << std::setw(2) << tmTime.tm_mday << "-" << std::setw(2) << yearRemainder << " ";
    ss << std::setw(2) << tmTime.tm_hour << ":" << std::setw(2) << tmTime.tm_min << ":" << std::setw(2) << tmTime.tm_sec << "." << std::setw(3) << millisecondsRemainder;

    return ss.str();
}

IP::String FormatTimeOfDay(SystemTimePoint timePoint)
{
    auto cTime = std::chrono::system_clock::to_time_t(timePoint);
    auto tmTime = localtime(cTime);

    // Grungy hack assumes that system clock "starts" with 0 milliseconds elapsed
    auto millisecondsElapsed = std::chrono::time_point_cast< std::chrono::milliseconds >(timePoint);
    auto millisecondsRemainder = millisecondsElapsed.time_since_epoch().count() % 1000;

    IP::StringStream ss;
    ss << std::setfill('0') << std::internal;
    ss << std::setw(2) << tmTime.tm_hour << ":" << std::setw(2) << tmTime.tm_min << ":" << std::setw(2) << tmTime.tm_sec << "." << std::setw(3) << millisecondsRemainder;

    return ss.str();
}

IP::String ConvertSystemTimeToFileSuffix(SystemTimePoint timePoint)
{
    auto cTime = std::chrono::system_clock::to_time_t(timePoint);
    auto tmTime = localtime(cTime);

    IP::StringStream ss;
    ss << std::setfill('0') << std::internal;
    ss << std::setw(4) << tmTime.tm_year + 1900 << "_" << std::setw(2) << tmTime.tm_mon + 1 << "_" << std::setw(2) << tmTime.tm_mday << "_" << std::setw(2) << tmTime.tm_hour;

    return ss.str();
}

SystemDuration GetElapsedSystemTime(void)
{
    auto now = std::chrono::system_clock::now();

    return now - SystemTimePoint::min();
}

double ConvertDurationToSeconds(SystemDuration duration)
{
    // Question: what is the best way of doing this precision-wise?
    double rawCount = static_cast< double >(duration.count());

    return rawCount / static_cast< double >(SystemDuration::period::den) * static_cast< double >(SystemDuration::period::num);
}

} // namespace Time
} // namespace IP
