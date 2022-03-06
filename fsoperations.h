#ifndef FSOPERATIONS_H
#define FSOPERATIONS_H

#include "DiskViewer_global.h"
#include <QList>
#include <QString>
#include <cinttypes>
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

enum class DeviceType {
};

struct VolumeInfo {
    QString name;
    QString link;
    char mountDrive = 0;
    VolumeInfo() = default;
    VolumeInfo(QString name, QString link, char mountDrive)
        : name(name)
        , link(link)
        , mountDrive(mountDrive)
    {
    }
    VolumeInfo(QString name, char mountDrive = '\0')
        : name(name)
        , mountDrive(mountDrive)
    {
    }
};

using VolumeInfoList = QList<VolumeInfo>;

struct DeviceInfo {
    std::string deviceName;
    VolumeInfoList volumes;
    DeviceType type;
};

using DeviceInfoList = QList<VolumeInfoList>;

class DISKVIEWER_EXPORT FSOperations final {
public:
    static FSOperations* instance();
    static void init();

    DeviceInfoList physicalDevices() const;
    QStringList dirEntryList(QStringList const& filters) const;
    HANDLE openFile(QString fileName, DWORD dAccess) const;

private:
    static FSOperations* m_instance;
    VolumeInfoList getVolumes() const;
    QString nativeReadLink(QString link) const;

    using NtOpenSymbolicLinkObjectT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PUNICODE_STRING);
    using NtQuerySumbolicLinkObjectT = NTSTATUS (*)(HANDLE, PUNICODE_STRING, PULONG);
    using NtOpenDirectoryObjectT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
    using NtQueryDirectoryObjectT = NTSTATUS (*)(HANDLE, POBJDIR_INFORMATION, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
    using RtlNtStatusToDosErrorT = DWORD (*)(NTSTATUS);
    using RtlInitUnicodeStringT = NTSTATUS (*)(PUNICODE_STRING, PWCHAR);
    using NtOpenFileT = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);

    NtOpenSymbolicLinkObjectT NtOpenSymbolicLinkObject;
    NtQuerySumbolicLinkObjectT NtQuerySumbolicLinkObject;
    NtOpenDirectoryObjectT NtOpenDirectoryObject;
    NtQueryDirectoryObjectT NtQueryDirectoryObject;
    RtlNtStatusToDosErrorT RtlNtStatusToDosError;
    RtlInitUnicodeStringT RtlInitUnicodeString;
    NtOpenFileT NtOpenFile;

    FSOperations();
};

}

#endif // FSOPERATIONS_H
