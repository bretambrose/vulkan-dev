#pragma once

#include <chrono>

#include <ip/core/memory/stl/String.h>

namespace IP
{
namespace Time
{

using SystemTimePoint = std::chrono::system_clock::time_point;
using SystemDuration = std::chrono::system_clock::duration;

SystemTimePoint GetCurrentSystemTime(void);
SystemDuration GetElapsedSystemTime(void);
bool GetFileLastModifiedTime(const IP::String &fileName, SystemTimePoint& lastModifiedTime);

double ConvertDurationToSeconds(SystemDuration duration);

IP::String FormatSystemTime(SystemTimePoint timePoint);
IP::String ConvertSystemTimeToFileSuffix(SystemTimePoint timePoint);

} // namespace Time
} // namespace IP
