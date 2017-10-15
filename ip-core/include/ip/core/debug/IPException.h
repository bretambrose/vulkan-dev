#pragma once

#include <stdexcept>
#include <ip/core/memory/stl/StringStream.h>

#include <ip/core/UnreferencedParam.h>

namespace IP
{

class IPException : public std::runtime_error
{
    public:

        using Base = std::runtime_error;

        IPException(const IP::String& message) :
            Base(message.c_str())
        {}

        ~IPException() {}

    private:

};


template <typename ...Args>
void FormatExceptionMessage(IP::StringStream& ss)
{
    IP_UNREFERENCED_PARAM(ss);
}

template <typename T, typename ...Args>
void FormatExceptionMessage(IP::StringStream& ss, T&& arg, Args... args)
{
    ss << arg;

    FormatExceptionMessage(ss, args...);
}

template <typename ...Args>
IP::String BuildIPExceptionMessage(Args... args)
{
    IP::StringStream ss;

    FormatExceptionMessage(ss, args...);

    return ss.str();
}

template <typename ...Args>
IPException BuildIPException(Args... args)
{
    return IPException(BuildIPExceptionMessage(args...));
}

} // namespace IP