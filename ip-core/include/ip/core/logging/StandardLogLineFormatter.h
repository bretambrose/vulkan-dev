#pragma once

#include <ip/core/logging/ILogLineFormatter.h>

namespace IP
{
namespace Logging
{

class StandardLogLineFormatter : public ILogLineFormatter
{
    public:

    virtual ~StandardLogLineFormatter() {}

    virtual void FormatLogLine(IP::StringStream& ss, LogEntry&& entry) const override;
};

} // namespace Logging
} // namespace IP
