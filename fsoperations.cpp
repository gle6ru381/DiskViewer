#include "fsoperations.h"
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
}

std::list<char> FSOperations::getAvailDrives() const
{
    std::list<char> result;
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < (int)sizeof(drives); i++) {
        if (drives & (1 << i))
            result.push_back('a' + 1);
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

    auto h = FindFirstVolumeW((wchar_t*)volumeName, std::size(volumeName));
    while (h != INVALID_HANDLE_VALUE) {
        VolumeInfo volume;
        volume.name = volumeName;
        volume.link = nativeReadLink(String(u"\\??\\") + (volumeName + 4));

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
        if (!FindNextVolumeW(h, (wchar_t*)volumeName, std::size(volumeName))) {
            h = INVALID_HANDLE_VALUE;
        }
    }
    FindVolumeClose(h);
    return result;
}
