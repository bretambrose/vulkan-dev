#pragma once

#include <stdint.h>

#include <ip/core/utils/TimeUtils.h>

namespace IP
{
namespace Render
{

class FrameRateLimiter
{
    public:

        FrameRateLimiter(uint32_t targetFrameRate);
        ~FrameRateLimiter();

        void ResetTargetFrameRate(uint32_t targetFrameRate);

        IP::Time::SystemDuration Service();

    private:

        IP::Time::SystemDuration ComputeFrameLength() const;

        uint32_t m_targetFrameRate;

        IP::Time::SystemTimePoint m_startTime;
        IP::Time::SystemTimePoint m_lastFrameCheckpoint;
};

} // namespace Render
} // namespace IP


