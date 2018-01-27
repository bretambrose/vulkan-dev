#include <ip/render/utilities/FrameRateLimiter.h>

namespace IP
{
namespace Render
{

FrameRateLimiter::FrameRateLimiter(uint32_t targetFrameRate) :
    m_targetFrameRate(targetFrameRate),
    m_startTime(IP::Time::GetCurrentSystemTime()),
    m_lastFrameCheckpoint()
{
    m_lastFrameCheckpoint = m_startTime - ComputeFrameLength();
}

FrameRateLimiter::~FrameRateLimiter()
{
}

void FrameRateLimiter::ResetTargetFrameRate(uint32_t targetFrameRate)
{
    m_targetFrameRate = targetFrameRate;
    m_lastFrameCheckpoint = IP::Time::GetCurrentSystemTime();
}

IP::Time::SystemDuration FrameRateLimiter::Service()
{
    IP::Time::SystemDuration frameLength = ComputeFrameLength();
    IP::Time::SystemTimePoint nextFrameTime = m_lastFrameCheckpoint + frameLength;
    IP::Time::SystemTimePoint currentTime = IP::Time::GetCurrentSystemTime();

    if (nextFrameTime <= currentTime)
    {
        if (currentTime - frameLength >= nextFrameTime)
        {
            // if we're badly late (more than an entire frame) then just sync the checkpoint to now
            m_lastFrameCheckpoint = currentTime;
        }
        else
        {
            // otherwise, if on time on mildly late, assume we can catch up and just advance the checkpoint by a single frame
            m_lastFrameCheckpoint += frameLength;
        }

        return IP::Time::SystemDuration::zero();
    }

    return nextFrameTime - currentTime;
}

IP::Time::SystemDuration FrameRateLimiter::ComputeFrameLength() const
{
    IP::Time::SystemDuration oneSecond(std::chrono::seconds(1));

    return oneSecond / m_targetFrameRate;
}


} // namespace Render
} // namespace IP