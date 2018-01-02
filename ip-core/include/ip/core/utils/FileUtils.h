#pragma once

#include <ip/core/memory/stl/Vector.h>
#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/StringStream.h>

namespace IP
{
namespace FileUtils
{
    // Directory functions
    bool DirectoryExists(const IP::String &path);
    bool CreateDirectoryByPath(const IP::String &path);
    void DeleteDirectory(const IP::String &path);
    const char *GetDirectorySeparator();
    bool IsDirectorySeparator(char value);

    // File functions
    void EnumerateMatchingFiles(const IP::String &pattern, IP::Vector< IP::String > &fileNames);
    bool DeleteFileByName(const IP::String &fileName);
    bool MoveFileByName(const IP::String &oldFilePath, const IP::String& newFilePath);

    IP::Vector<uint8_t> LoadFileData(const IP::String& fileName);

    // Path functions
    IP::String ExtractFilename(const IP::String& filePath);
    IP::String ExtractPath(const IP::String& filePath);
    void SplitPath(const IP::String& path, IP::Vector<IP::String>& pathComponents);
    IP::String JoinPath(const IP::Vector<IP::String>& pathComponents);

    
    void JoinPathAux(IP::StringStream& ss);
    void JoinPathAux(IP::StringStream& ss, const IP::String& component);

    template<typename ...Args>
    void JoinPathAux(IP::StringStream& ss, const IP::String& firstComponent, Args... otherComponents)
    {
        ss << firstComponent;
        if (!firstComponent.empty() && !IsDirectorySeparator(firstComponent[firstComponent.size() - 1]))
        {
            ss << GetDirectorySeparator();
        }

        JoinPathAux(ss, otherComponents...);
    }

    template<typename ...Args>
    IP::String JoinPath(Args... args)
    {
        IP::StringStream ss;

        JoinPathAux(ss, args...);

        return ss.str();
    }

} // namespace FileUtils
} // namespace IP
