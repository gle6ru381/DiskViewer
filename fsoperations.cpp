#include "fsoperations.h"
#include "nativefunc.h"
#include "ntdir.h"
#include <codecvt>
#include <cwchar>
#include <iostream>
#include <windows.h>

#include <debugapi.h>
#include <fileapi.h>

using namespace dv;
using OBJECT_ATTRIBUTES = nt::OBJECT_ATTRIBUTES;
using UNICODE_STRING = nt::UNICODE_STRING;
using POBJDIR_INFORMATION = nt::POBJDIR_INFORMATION;

String dv::formatError(DWORD error)
{
    String result(256, 0);
    auto size = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)result.data(),
            result.size(),
            nullptr);
    result.resize(size);
    return result;
}

static String nativeReadLink(const String& link)
{
    UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;
    HANDLE hObject;
    auto f = NativeFunc::instance();

    f->RtlInitUnicodeString(&uname, (wchar_t*)link.c_str());
    InitializeObjectAttributes(
            &objAttrs, &uname, OBJ_CASE_INSENSITIVE, 0, nullptr);

    auto status = f->NtOpenSymbolicLinkObject(&hObject, 0x0001, &objAttrs);
    if (NT_SUCCESS(status)) {
        uname.Length = 0;
        char16_t data[256];
        uname.MaximumLength = sizeof(data);
        uname.Buffer = (wchar_t*)data;

        status = f->NtQuerySymbolicLinkObject(hObject, &uname, nullptr);
        if (NT_SUCCESS(status)) {
            CloseHandle(hObject);
            return String(data, uname.Length / 2);
        }
    }
    CloseHandle(hObject);
    return String();
}

static DWORD ctlCode(DWORD deviceType, DWORD func, DWORD method, DWORD access)
{
    return (deviceType << 16) | (access << 14) | (func << 2) | (method);
}

static HANDLE nativeCreateFile(const String& fName, DWORD dAccess, DWORD& error)
{
    nt::UNICODE_STRING uname;
    nt::OBJECT_ATTRIBUTES objAttrs;
    nt::IO_STATUS_BLOCK status;
    HANDLE result;
    auto f = NativeFunc::instance();

    f->RtlInitUnicodeString(&uname, (wchar_t*)fName.c_str());
    InitializeObjectAttributes(
            &objAttrs, &uname, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    auto err = f->NtOpenFile(
            &result, dAccess | SYNCHRONIZE, &objAttrs, &status, 0, 0x00000020);
    if (!NT_SUCCESS(err))
        result = INVALID_HANDLE_VALUE;

    error = f->RtlNtStatusToDosError(err);

    return result;
}

static bool testDevice(const String& deviceName)
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

std::vector<DirToken> dv::dirEntryTokenList(
        StringView dir, const std::list<std::pair<std::wregex, int>>& tokenDict)
{
    std::vector<DirToken> result;
    NtDir d(dir);

    if (d.open()) {
        wchar_t data[256] = {};
        auto dirInfo = POBJDIR_INFORMATION(data);
        while (d.read(dirInfo, sizeof(data))) {
            DirToken dToken;
            std::wstring_view str(
                    dirInfo->Name.Buffer, dirInfo->Name.Length / 2);
            for (auto const& [regex, resToken] : tokenDict) {
                if (std::regex_search(str.begin(), str.end(), regex)) {
                    dToken.name = StringView((char16_t*)str.data(), str.size());
                    dToken.token = resToken;
                    result.emplace_back(dToken);
                    break;
                }
            }
        }
    }

    return result;
}

StringList dv::dirEntryList(StringView dirName, std::wregex const& filter)
{
    StringList result;
    NtDir dir(dirName);

    if (dir.open()) {
        wchar_t data[256] = {};
        auto dirInfo = POBJDIR_INFORMATION(data);
        while (dir.read(dirInfo, sizeof(data))) {
            std::wstring_view str(
                    dirInfo->Name.Buffer, dirInfo->Name.Length / 2);
            if (std::regex_search(str.begin(), str.end(), filter))
                result.emplace_back(String((char16_t*)str.data(), str.size()));
        }
    }
    return result;
}

NtBlockDeviceList dv::getNtBlockDevices()
{
    NtBlockDeviceList result;

    auto data = dirEntryTokenList(
            u"\\Device",
            {{std::wregex(L"Harddisk\\d+"), (int)DeviceType::Harddisk},
             {std::wregex(L"Floppy\\d+"), (int)DeviceType::Floppy},
             {std::wregex(L"CdRom\\d+"), (int)DeviceType::CdRom}});

    for (int i = 0; i < data.size(); i++) {
        NtBlockDevice device;
        auto const& file = data[i].name;
        device.type = (DeviceType)data[i].token;

        if (device.type == DeviceType::Harddisk) {
            device.name = u"\\Device\\" + file;
            auto partitions
                    = dirEntryList(device.name, std::wregex(L"Partition\\d+"));
            for (auto const& part : partitions) {
                NtDevicePartition partition;
                partition.name = part;
                String fullName = device.name + u'\\' + partition.name;
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

std::list<char> dv::getAvailDrives()
{
    std::list<char> result;
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < (int)sizeof(drives); i++) {
        if (drives & (1 << i))
            result.push_back('a' + i);
    }
    return result;
}

HANDLE dv::openFile(StringView fileName, DWORD dAccess)
{
    DWORD shareAccess = 0;
    //    if (dAccess & GENERIC_READ)
    //        shareAccess |= FILE_SHARE_READ;
    //    if (dAccess & GENERIC_WRITE)
    //        shareAccess |= FILE_SHARE_WRITE;
    return CreateFileW(
            toWide(fileName).data(),
            dAccess,
            shareAccess,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
}

VolumeInfoList dv::getVolumes()
{
    VolumeInfoList result;
    auto drives = getAvailDrives();
    char16_t volumeName[256];

    HANDLE h = FindFirstVolumeW((wchar_t*)volumeName, std::size(volumeName));
    while (h != INVALID_HANDLE_VALUE) {
        VolumeInfo volume;
        volume.name = volumeName;
        String linkName = u"\\??\\";
        linkName += std::u16string_view(volumeName + 4, volume.name.size() - 5);
        volume.link = nativeReadLink(linkName);

        for (auto drive = drives.begin(); drive != drives.end(); drive++) {
            String driveString = (char16_t)*drive + String(u":\\");
            char16_t buff[256];

            if (GetVolumeNameForVolumeMountPointW(
                        (wchar_t*)driveString.c_str(),
                        (wchar_t*)buff,
                        std::size(buff))) {
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
