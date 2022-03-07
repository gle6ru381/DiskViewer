#ifndef FSOPERATIONS_H
#define FSOPERATIONS_H

#include "DiskViewer_global.h"
#include <cinttypes>
#include <list>
#include <string>
#include <vector>
#include <windows.h>

#include <ntdef.h>

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _OBJDIR_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJDIR_INFORMATION, *POBJDIR_INFORMATION;

namespace dv {

using String = std::u16string;
using StringList = std::vector<String>;

enum class DeviceType {
};

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

using VolumeInfoList = std::vector<VolumeInfo>;

struct DeviceInfo {
    std::string deviceName;
    VolumeInfoList volumes;
    DeviceType type;
};

using DeviceInfoList = std::vector<VolumeInfoList>;

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
    StringList dirEntryList(StringList const& filters) const;
    HANDLE openFile(String const& fileName, DWORD dAccess) const;
    HANDLE openFile(wchar_t* fileName, DWORD dAccess) const;
    VolumeInfoList getVolumes() const;

private:
    static FSOperations* m_instance;
    String nativeReadLink(String const& link) const;
    std::list<char> getAvailDrives() const;

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
