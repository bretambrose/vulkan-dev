#pragma once

#include <ip/core/logging/ILogger.h>

#include <ip/core/memory/stl/Vector.h>

namespace IP
{
namespace Logging
{

class CompositeLogger : public ILogger
{
    public:
        CompositeLogger(IP::Vector<IP::UniquePtr<ILogger>>&& loggers);
        virtual ~CompositeLogger();

        virtual void Log(LogEntry&& entry) override;

    private:

        IP::Vector<IP::UniquePtr<ILogger>> m_loggers;

};

} // namespace Logging
} // namespace IP
