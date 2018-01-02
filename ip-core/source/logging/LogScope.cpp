#include <ip/core/logging/LogScope.h>

#include <ip/core/logging/LogSystem.h>

namespace IP
{
namespace Logging
{

LogScope::LogScope(IP::UniquePtr<ILogger> &&logger) :
    m_logger(std::move(logger))
{
    IP::Logging::Initialize(m_logger.get());
}

LogScope::~LogScope()
{
    IP::Logging::Shutdown();
}

}
}