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
IP::String BuildIPExceptionMessage(const char* fileName, uint32_t lineNumber, Args... args)
{
    IP::StringStream ss;

    ss << "Exception (" << fileName << ": " << lineNumber << ")" << std::endl;
    FormatExceptionMessage(ss, args...);

    return ss.str();
}

template <typename ...Args>
IPException BuildIPException(const char* fileName, uint32_t lineNumber, Args... args)
{
    return IPException(BuildIPExceptionMessage(fileName, lineNumber, args...));
}

#define THROW_IP_EXCEPTION( ... ) throw BuildIPException( __FILE__, __LINE__, __VA_ARGS__ );

} // namespace IP