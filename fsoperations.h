#ifndef FSOPERATIONS_H
#define FSOPERATIONS_H

#include "DiskViewer_global.h"
#include "disktypes.h"
#include "nativetypes.h"
#include <regex>
#include <windows.h>

namespace dv {

class DISKVIEWER_EXPORT FSOperations final {
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

    struct VolumeInfo {
        String name;
        String link;
        char mountDrive = 0;
        VolumeInfo() = default;
        VolumeInfo(String const& name, String const& link, char mountDrive)
            : name(name), link(link), mountDrive(mountDrive)
        {
        }
        VolumeInfo(String const& name, char mountDrive = '\0')
            : name(name), mountDrive(mountDrive)
        {
        }
    };
    using VolumeInfoList = std::list<VolumeInfo>;

public:
    static FSOperations* instance()
    {
        if (!m_instance)
            m_instance = new FSOperations;
        return m_instance;
    }

    StringList
    dirEntryList(String const& dir, std::wregex const& filters) const;
    HANDLE openFile(String const& fileName, DWORD dAccess) const;
    HANDLE openFile(wchar_t* fileName, DWORD dAccess) const;
    VolumeInfoList getVolumes() const;
    NtBlockDeviceList getNtBlockDevices() const;
    std::list<char> getAvailDrives() const;
    static String formatMessage(DWORD error);

private:
    static FSOperations* m_instance;
    String nativeReadLink(String const& link) const;
    bool testDevice(String const& deviceName) const;
    static DWORD
    ctlCode(DWORD deviceType, DWORD func, DWORD method, DWORD access);
    HANDLE
    nativeCreateFile(String const& fName, DWORD dAccess, DWORD& error) const;

    nt::NtOpenSymbolicLinkObjectT NtOpenSymbolicLinkObject;
    nt::NtQuerySymbolicLinkObjectT NtQuerySymbolicLinkObject;
    nt::NtOpenDirectoryObjectT NtOpenDirectoryObject;
    nt::NtQueryDirectoryObjectT NtQueryDirectoryObject;
    nt::RtlNtStatusToDosErrorT RtlNtStatusToDosError;
    nt::RtlInitUnicodeStringT RtlInitUnicodeString;
    nt::NtOpenFileT NtOpenFile;

    FSOperations();
};

}

#endif // FSOPERATIONS_H
