#include <ip/render/GlfwError.h>

namespace IP
{
namespace Render
{

std::mutex GlfwErrorTracker::m_errorTableLock;
IP::UnorderedMap<std::thread::id, GlfwError> GlfwErrorTracker::m_lastErrors;

bool GlfwErrorTracker::FetchLastError(GlfwError& error)
{
    std::lock_guard<std::mutex> tableLock(m_errorTableLock);

    const auto& iter = m_lastErrors.find(std::this_thread::get_id());
    if (iter == m_lastErrors.cend()) {
        return false;
    }

    error = iter->second;
    return true;
}

void GlfwErrorTracker::GlfwErrorCallback(int errorCode, const char* message)
{
    std::lock_guard<std::mutex> tableLock(m_errorTableLock);

    m_lastErrors[std::this_thread::get_id()] = GlfwError(errorCode, message);
}

} // namespace Render
} // namespace IP