#include "fsoperations.h"
#include <iostream>
#include <windows.h>

#include <fileapi.h>
#include <libloaderapi.h>

using namespace dv;

FSOperations* FSOperations::m_instance = nullptr;

FSOperations::FSOperations()
{
    auto lib = LoadLibraryW((wchar_t const*)u"Ntdll.dll");
    NtOpenSymbolicLinkObject = (NtOpenSymbolicLinkObjectT)GetProcAddress(lib, "NtOpenSymbolicLinkObject");
    NtQuerySymbolicLinkObject = (NtQuerySymbolicLinkObjectT)GetProcAddress(lib, "NtQuerySymbolicLinkObject");
    NtOpenDirectoryObject = (NtOpenDirectoryObjectT)GetProcAddress(lib, "NtOpenDirectoryObject");
    NtQueryDirectoryObject = (NtQueryDirectoryObjectT)GetProcAddress(lib, "NtQueryDirectoryObject");
    RtlNtStatusToDosError = (RtlNtStatusToDosErrorT)GetProcAddress(lib, "RtlNtStatusToDosError");
    RtlInitUnicodeString = (RtlInitUnicodeStringT)GetProcAddress(lib, "RtlInitUnicodeString");
    NtOpenFile = (NtOpenFileT)GetProcAddress(lib, "NtOpenFile");
}

String FSOperations::nativeReadLink(const String& link) const
{
#if (_WIN32_WINNT >= 0x0600)
    auto file = openFile(link, 0);
    String result;
    result.resize(1024);
    auto len = GetFinalPathNameByHandleW(file, (wchar_t*)result.data(), result.size(), FILE_NAME_NORMALIZED);
    result.resize(len);
    return result;
#else
    UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;
    HANDLE hObject;

    RtlInitUnicodeString(&uname, (wchar_t*)link.c_str());
    InitializeObjectAttributes(&objAttrs, &uname, OBJ_CASE_INSENSITIVE, 0, nullptr);

    auto status = NtOpenSymbolicLinkObject(&hObject, 0x0001, &objAttrs);
    if (NT_SUCCESS(status)) {
        uname.Length = 0;
        char16_t data[1024];
        uname.MaximumLength = sizeof(data);
        uname.Buffer = (wchar_t*)data;

        status = NtQuerySymbolicLinkObject(hObject, &uname, nullptr);
        if (NT_SUCCESS(status)) {
            CloseHandle(hObject);
            return String(data, uname.Length / 2);
        }
    }
    CloseHandle(hObject);
    return String();
#endif
}

std::list<char> FSOperations::getAvailDrives() const
{
    std::list<char> result;
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < (int)sizeof(drives); i++) {
        if (drives & (1 << i))
            result.push_back('a' + i);
    }
    return result;
}

HANDLE FSOperations::openFile(String const& fileName, DWORD dAccess) const
{
    return openFile((wchar_t*)fileName.c_str(), dAccess);
}

HANDLE FSOperations::openFile(wchar_t* fileName, DWORD dAccess) const
{
    return CreateFileW(fileName, dAccess, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
}

VolumeInfoList
FSOperations::getVolumes() const
{
    VolumeInfoList result;
    auto drives = getAvailDrives();
    char16_t volumeName[1024];

    HANDLE h = FindFirstVolumeW((wchar_t*)volumeName, std::size(volumeName));
    while (h != INVALID_HANDLE_VALUE) {
        VolumeInfo volume;
        volume.name = volumeName;
        String linkName = u"\\??\\";
        linkName += std::u16string_view(volumeName + 4, volume.name.size() - 5);
        volume.link = nativeReadLink(linkName);

        for (auto drive = drives.begin(); drive != drives.end(); drive++) {
            String driveString = (char16_t)*drive + String(u":\\");
            char16_t buff[1024];

            if (GetVolumeNameForVolumeMountPointW((wchar_t*)driveString.c_str(), (wchar_t*)buff, std::size(buff))) {
                if (volume.name == buff) {
                    volume.mountDrive = *drive;
                    drives.erase(drive);
                    break;
                }
            }
        }
        result.emplace_back(volume);
        if (!FindNextVolumeW(h, (wchar_t*)volumeName, std::size(volumeName))) {
            FindVolumeClose(h);
            h = INVALID_HANDLE_VALUE;
        }
    }
    return result;
}
