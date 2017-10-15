#pragma once

#include <mutex>
#include <thread>
#include <ip/core/memory/stl/UnorderedMap.h>

namespace IP
{

struct GlfwError 
{
    public:
        GlfwError(int errorCode, const char* message) :
            m_message(message),
            m_errorCode(errorCode)
        {}

        GlfwError() :
            m_message(""),
            m_errorCode(0)
        {}

        GlfwError(const GlfwError& rhs) :
            m_message(rhs.m_message),
            m_errorCode(rhs.m_errorCode)
        {}

        std::string m_message;
        int m_errorCode;

};

class GlfwErrorTracker
{
    public:

        static bool FetchLastError(GlfwError& error);
        static void GlfwErrorCallback(int errorCode, const char* message);

    private:

        static std::mutex m_errorTableLock;
        static IP::UnorderedMap<std::thread::id, GlfwError> m_lastErrors;

};

} // namespace IP
