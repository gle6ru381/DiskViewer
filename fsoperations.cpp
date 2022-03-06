#include "fsoperations.h"

#include <fileapi.h>
#include <windows.h>

using namespace dv;

FSOperations* FSOperations::m_instance = nullptr;

FSOperations::FSOperations()
{
}

HANDLE FSOperations::openFile(QString fileName, DWORD dAccess) const
{
    return CreateFileW((const wchar_t*)fileName.utf16(), dAccess, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
}

VolumeInfoList
FSOperations::getVolumes() const
{
    VolumeInfoList result;

    char volumeName[1024];
    auto h = FindFirstVolumeA(volumeName, sizeof(volumeName));
    while (h != INVALID_HANDLE_VALUE) {
        auto fHandle = openFile(volumeName, 0);
    }

    for (char drive = 'a'; drive <= 'z'; drive++) {
        std::string driveString = drive + std::string(":\\");
        char buff[1024];
        if (GetVolumeNameForVolumeMountPointA(
                driveString.data(), buff, sizeof(buff))) {
            if (buff[0] != '\0') {
                result.push_back(VolumeInfo(buff, drive));
            }
        }
    }
    return result;
}
