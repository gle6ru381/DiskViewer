#ifndef FSOPERATIONS_H
#define FSOPERATIONS_H

#include "DiskViewer_global.h"
#include "disktypes.h"
#include "nativetypes.h"
#include <regex>
#include <windows.h>

namespace dv {

StringList dirEntryList(StringView dir, std::wregex const& filters);
struct DirToken {
    String name;
    int token;
};
std::vector<DirToken> dirEntryTokenList(
        StringView dir,
        std::list<std::pair<std::wregex, int>> const& tokenDict);
HANDLE openFile(StringView fileName, DWORD dAccess);
void closeFile(HANDLE handle);
long fileWrite(HANDLE file, void* data, size_t size);
long fileRead(HANDLE file, void* data, size_t size);
VolumeInfoList getVolumes();
NtBlockDeviceList getNtBlockDevices();
std::list<char> getAvailDrives();
String formatError(DWORD error);
}

#endif // FSOPERATIONS_H
