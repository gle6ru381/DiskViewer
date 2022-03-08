#ifndef FSOPERATIONS_H
#define FSOPERATIONS_H

#include "DiskViewer_global.h"
#include <cinttypes>
#include <list>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace nt {
#include <ntdef.h>
}

using UNICODE_STRING = nt::UNICODE_STRING;
using PUNICODE_STRING = nt::PCUNICODE_STRING;
using OBJECT_ATTRIBUTES = nt::OBJECT_ATTRIBUTES;
using POBJECT_ATTRIBUTES = nt::POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _OBJDIR_INFORMATION {
    nt::UNICODE_STRING Name;
    nt::UNICODE_STRING TypeName;
} OBJDIR_INFORMATION, *POBJDIR_INFORMATION;

namespace dv {

using String = std::u16string;
using StringList = std::vector<String>;

struct VolumeInfo {
    String name;
    String link;
    char mountDrive = 0;
    VolumeInfo() = default;
    VolumeInfo(String const& name, String const& link, char mountDrive)
        : name(name)
        , link(link)
        , mountDrive(mountDrive)
    {
    }
    VolumeInfo(String const& name, char mountDrive = '\0')
        : name(name)
        , mountDrive(mountDrive)
    {
    }
};

using VolumeInfoList = std::list<VolumeInfo>;

struct Partition {
    String name;
    String link;
    String partitionName;
    char mountDrive;
};

using PartitionList = std::unordered_map<uint32_t, Partition>;

struct DeviceInfo {
    String deviceName;
    PartitionList partitions;
};

using DeviceInfoList = std::vector<DeviceInfo>;

struct NtDevicePartition {
    String name;
    String link;
};

using NtDevicePartitionList = std::list<NtDevicePartition>;

struct NtBlockDevice {
    String name;
    String link;
    NtDevicePartitionList partitions;
};

using NtBlockDeviceList = std::vector<NtBlockDevice>;

class DISKVIEWER_EXPORT FSOperations final {
public:
    static FSOperations* instance()
    {
        return m_instance;
    }
    static void init()
    {
        if (!m_instance)
            m_instance = new FSOperations;
    }

    DeviceInfoList physicalDevices() const;
    StringList dirEntryList(String const& dir, std::wregex const& filters) const;
    HANDLE openFile(String const& fileName, DWORD dAccess) const;
    HANDLE openFile(wchar_t* fileName, DWORD dAccess) const;
    VolumeInfoList getVolumes() const;
    NtBlockDeviceList getNtBlockDevices() const;

private:
    static FSOperations* m_instance;
    String nativeReadLink(String const& link) const;
    std::list<char> getAvailDrives() const;
    static String formatMessage(DWORD error);
    bool testDevice(String const& deviceName) const;
    static bool startsWith(std::u16string_view sv);
    static DWORD ctlCode(DWORD deviceType, DWORD func, DWORD method, DWORD access);
    HANDLE nativeCreateFile(String const& fName, DWORD dAccess, DWORD& error) const;

    using NtOpenSymbolicLinkObjectT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
    using NtQuerySymbolicLinkObjectT = NTSTATUS (*)(HANDLE, PUNICODE_STRING, PULONG);
    using NtOpenDirectoryObjectT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
    using NtQueryDirectoryObjectT = NTSTATUS (*)(HANDLE, POBJDIR_INFORMATION, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
    using RtlNtStatusToDosErrorT = DWORD (*)(NTSTATUS);
    using RtlInitUnicodeStringT = NTSTATUS (*)(PUNICODE_STRING, PWCHAR);
    using NtOpenFileT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);

    NtOpenSymbolicLinkObjectT NtOpenSymbolicLinkObject;
    NtQuerySymbolicLinkObjectT NtQuerySymbolicLinkObject;
    NtOpenDirectoryObjectT NtOpenDirectoryObject;
    NtQueryDirectoryObjectT NtQueryDirectoryObject;
    RtlNtStatusToDosErrorT RtlNtStatusToDosError;
    RtlInitUnicodeStringT RtlInitUnicodeString;
    NtOpenFileT NtOpenFile;

    FSOperations();
};

}

#endif // FSOPERATIONS_H
