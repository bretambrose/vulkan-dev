#include <ip/core/utils/FileUtils.h>

#include <ip/core/debug/IPException.h>
#include <ip/core/memory/stl/Stream.h>

#include <fstream>

namespace IP
{
namespace FileUtils
{

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

} // namespace FileUtils
} // namespace IP
