#include <ip/core/utils/FileUtils.h>

#include <ip/core/debug/IPException.h>
#include <ip/core/memory/stl/Stream.h>

#include <fstream>

#ifdef _WINDOWS
#include <Windows.h>
#include <Shlwapi.h>
#endif // _WINDOWS

namespace IP
{
namespace FileUtils
{

static const char* s_windowsPathSeparator = "\\";
static const char* s_unixPathSeparator = "/";
static const char* s_pathSeparators = "\\/";
static const IP::String s_pathSeparatorsString(s_pathSeparators);

IP::Vector<uint8_t> LoadFileData(const IP::String& fileName)
{
    IP::IFStream inputFile(fileName.c_str(), std::ios::ate | std::ios::binary);
    if (!inputFile.good())
    {
        THROW_IP_EXCEPTION("Error opening file: ", fileName);
    }

    size_t fileLength = inputFile.tellg();
    IP::Vector<uint8_t> fileData(fileLength);

    inputFile.seekg(0);
    inputFile.read(reinterpret_cast<char *>(fileData.data()), fileLength);
    inputFile.close();

    return fileData;
}

static void SplitPathAndFilename(const IP::String& filePath, IP::String& path, IP::String& filename)
{
    path = "";
    filename = "";

    if (filePath.empty())
    {
        return;
    }

    size_t separatorCount = s_pathSeparatorsString.size();
    for (size_t i = filePath.size(); i > 0; --i)
    {
        size_t index = i - 1;
        for (size_t j = 0; j < separatorCount; ++j)
        {
            if (filePath[index] == s_pathSeparators[j])
            {
                path = IP::String(filePath.cbegin(), filePath.cbegin() + index);
                if (i != filePath.size())
                {
                    filename = IP::String(filePath.cbegin() + index + 1, filePath.cend());
                }
                return;
            }
        }
    }

    filename = filePath;
}

IP::String ExtractPath(const IP::String& filePath)
{
    IP::String path;
    IP::String filename;
    SplitPathAndFilename(filePath, path, filename);

    return path;
}

IP::String ExtractFilename(const IP::String& filePath)
{
    IP::String path;
    IP::String filename;
    SplitPathAndFilename(filePath, path, filename);

    return filename;
}

bool IsDirectorySeparator(char value)
{
    return strchr(s_pathSeparators, value) != nullptr;
}

void SplitPath(const IP::String& path, IP::Vector<IP::String>& pathComponents)
{
    pathComponents.clear();

    const char *pathStart = path.c_str();
    size_t componentStartIndex = 0;
    size_t pathEndIndex = path.size();
    while (componentStartIndex < pathEndIndex)
    {
        size_t componentLength = strcspn(pathStart + componentStartIndex, s_pathSeparators);
        if (componentLength > 0)
        {
            pathComponents.push_back(IP::String(pathStart + componentStartIndex, componentLength - 1));
        }

        componentStartIndex += componentLength + 1;
    }
}

IP::String JoinPath(const IP::Vector<IP::String>& pathComponents)
{
    IP::String filePath;

    size_t componentCount = pathComponents.size();
    for (uint32_t i = 0; i < componentCount; ++i)
    {
        const auto& component = pathComponents[i];

        filePath += component;
        if (i + 1 < componentCount && !component.empty() && !IsDirectorySeparator(component[component.size() - 1]))
        {
            filePath += GetDirectorySeparator();
        }
    }

    return filePath;
}

void JoinPathAux(IP::StringStream& ss)
{
    IP_UNREFERENCED_PARAM(ss);
}

void JoinPathAux(IP::StringStream& ss, const IP::String& component)
{
    ss << component;
}

#ifdef _WINDOWS
const char *GetDirectorySeparator()
{
    return s_windowsPathSeparator;
}

bool DirectoryExists(const IP::String &path)
{
    DWORD attribs = ::GetFileAttributes(path.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    return (attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool CreateDirectoryByPath(const IP::String &path)
{
    return ::CreateDirectory(path.c_str(), nullptr) != 0;
}


void DeleteDirectory(const IP::String &path)
{
    ::RemoveDirectory(path.c_str());
}


void EnumerateMatchingFiles(const IP::String &pattern, IP::Vector< IP::String > &fileNames)
{
    fileNames.clear();

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA findInfo;

    fileHandle = ::FindFirstFile(pattern.c_str(), &findInfo);
    if (INVALID_HANDLE_VALUE == fileHandle)
    {
        return;
    }

    do
    {
        if ((findInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            fileNames.push_back(IP::String(findInfo.cFileName));
        }
    } while (::FindNextFile(fileHandle, &findInfo) != 0);

    ::FindClose(fileHandle);
}


bool DeleteFileByName(const IP::String &fileName)
{
    if (::DeleteFile(fileName.c_str()) == 0)
    {
        DWORD error = GetLastError();
        error = error;
        return false;
    }

    return true;
}

bool MoveFileByName(const IP::String &oldFilePath, const IP::String& newFilePath)
{
    if (::MoveFile(oldFilePath.c_str(), newFilePath.c_str()) == 0)
    {
        DWORD error = GetLastError();
        error = error;
        return false;
    }

    return true;
}

#endif



} // namespace FileUtils
} // namespace IP
