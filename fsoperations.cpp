#include "fsoperations.h"
#include <codecvt>
#include <iostream>
#include <windows.h>

#include <debugapi.h>
#include <fileapi.h>
#include <libloaderapi.h>

using namespace dv;

FSOperations* FSOperations::m_instance = nullptr;

FSOperations::FSOperations()
{
    auto lib = LoadLibraryW((wchar_t const*)u"Ntdll.dll");
    NtOpenSymbolicLinkObject = (NtOpenSymbolicLinkObjectT)GetProcAddress(
        lib, "NtOpenSymbolicLinkObject");
    NtQuerySymbolicLinkObject = (NtQuerySymbolicLinkObjectT)GetProcAddress(
        lib, "NtQuerySymbolicLinkObject");
    NtOpenDirectoryObject = (NtOpenDirectoryObjectT)GetProcAddress(lib, "NtOpenDirectoryObject");
    NtQueryDirectoryObject = (NtQueryDirectoryObjectT)GetProcAddress(lib, "NtQueryDirectoryObject");
    RtlNtStatusToDosError = (RtlNtStatusToDosErrorT)GetProcAddress(lib, "RtlNtStatusToDosError");
    RtlInitUnicodeString = (RtlInitUnicodeStringT)GetProcAddress(lib, "RtlInitUnicodeString");
    NtOpenFile = (NtOpenFileT)GetProcAddress(lib, "NtOpenFile");
}

String FSOperations::formatMessage(DWORD error)
{
    String result(1024, 0);
    auto size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)result.data(), result.size(),
        nullptr);
    result.resize(size);
    return result;
}

String FSOperations::nativeReadLink(const String& link) const
{
    UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;
    HANDLE hObject;

    RtlInitUnicodeString(&uname, (wchar_t*)link.c_str());
    InitializeObjectAttributes(&objAttrs, &uname, OBJ_CASE_INSENSITIVE, 0,
        nullptr);

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

DWORD FSOperations::ctlCode(DWORD deviceType, DWORD func, DWORD method, DWORD access)
{
    return (deviceType << 16) | (access << 14) | (func << 2) | (method);
}

HANDLE FSOperations::nativeCreateFile(const String& fName, DWORD dAccess, DWORD& error) const
{
    UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;
    IO_STATUS_BLOCK status;
    HANDLE result;

    RtlInitUnicodeString(&uname, (wchar_t*)fName.c_str());
    InitializeObjectAttributes(&objAttrs, &uname, OBJ_CASE_INSENSITIVE,
        nullptr, nullptr);

    auto err = NtOpenFile(&result, dAccess | SYNCHRONIZE, &objAttrs, &status, 0, 0x00000020);
    if (!NT_SUCCESS(err))
        result = INVALID_HANDLE_VALUE;

    error = RtlNtStatusToDosError(err);

    return result;
}

bool FSOperations::testDevice(const String& deviceName) const
{
    bool result = false;

    DWORD err;
    auto h = nativeCreateFile(deviceName, GENERIC_READ, err);

    if (h != INVALID_HANDLE_VALUE) {
        result = true;
    } else {
        switch (err) {
        case ERROR_SHARING_VIOLATION:
            result = true;
            break;
        }
    }
    return result;
}

StringList FSOperations::dirEntryList(String const& dir, const std::wregex& filters) const
{
    StringList result;
    UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;
    HANDLE h;

    RtlInitUnicodeString(&uname, (wchar_t*)dir.c_str());
    InitializeObjectAttributes(&objAttrs, &uname, OBJ_CASE_INSENSITIVE, 0, nullptr);
    auto status = NtOpenDirectoryObject(&h, STANDARD_RIGHTS_READ | 0x0001, &objAttrs);

    if (NT_SUCCESS(status)) {
        ULONG idx = 0;
        while (true) {
            wchar_t data[1024] = {};
            auto dirInfo = POBJDIR_INFORMATION(data);
            status = NtQueryDirectoryObject(h, dirInfo, std::size(data), true, false, &idx, nullptr);

            if (NT_SUCCESS(status)) {
                std::wstring_view str(dirInfo->Name.Buffer, dirInfo->Name.Length / 2);
                if (std::regex_search(str.begin(), str.end(), filters))
                    result.emplace_back(String((char16_t*)dirInfo->Name.Buffer, dirInfo->Name.Length / 2));
            } else
                break;
        }
        CloseHandle(h);
    }
    return result;
}

NtBlockDeviceList FSOperations::getNtBlockDevices() const
{
    NtBlockDeviceList result;

    auto data = dirEntryList(u"\\Device", std::wregex(L"(CdRom|Floppy|Harddisk)\\d+"));

    for (int i = 0; i < data.size(); i++) {
        NtBlockDevice device;
        auto const& file = data[i];

        if (file._Starts_with(u"Harddisk")) {
            device.name = u"\\Device\\" + file;
            auto partitions = dirEntryList(device.name, std::wregex(L"Partition\\d+"));
            for (auto const& part : partitions) {
                Partition partition;
                partition.name = part;
                String fullName = device.name + u'\\' + partition.name;
                OutputDebugStringW((wchar_t*)fullName.c_str());
                OutputDebugStringW(L"\n");
                if (testDevice(fullName)) {
                    partition.link = nativeReadLink(fullName);
                }
                device.partitions.emplace_back(partition);
            }
        } else {
            device.name = u"\\Device\\" + file;
            if (testDevice(device.name)) {
                auto link = nativeReadLink(device.name);
                if (!link.empty())
                    device.link = u"\\\\?" + link;
            }
        }
        result.emplace_back(device);
    }

    return result;
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
    DWORD shareAccess = 0;
    //    if (dAccess & GENERIC_READ)
    //        shareAccess |= FILE_SHARE_READ;
    //    if (dAccess & GENERIC_WRITE)
    //        shareAccess |= FILE_SHARE_WRITE;
    return CreateFileW(fileName, dAccess, shareAccess, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
}

VolumeInfoList FSOperations::getVolumes() const
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

            if (GetVolumeNameForVolumeMountPointW((wchar_t*)driveString.c_str(),
                    (wchar_t*)buff, std::size(buff))) {
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
