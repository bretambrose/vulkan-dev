
#pragma once

#include <ip/core/memory/stl/String.h>

#include <ip/core/memory/stl/Vector.h>

#include <functional>

namespace IP
{
namespace StringUtils
{

    template<typename T, typename Fn>
    IP::String ToString(const IP::Vector<T>& items, const char *separator, Fn itemFunction)
    {
        IP::StringStream ss;

        for (uint32_t i = 0; i < items.size(); ++i)
        {
            ss << itemFunction(items[i]);
            if (i + 1 < items.size())
            {
                ss << separator;
            }
        }

        return ss.str();
    }

    IP::String ToString(const IP::Vector<IP::String>& items, const char *separator);

} // namespace StringUtils
} // namespace IP

