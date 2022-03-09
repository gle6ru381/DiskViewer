#ifndef DISKTYPES_H
#define DISKTYPES_H

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace dv {
using String = std::u16string;
using StringView = std::u16string_view;

inline wchar_t* toWide(String& str)
{
    return (wchar_t*)str.data();
}

inline std::wstring_view toWide(StringView str)
{
    return std::wstring_view((wchar_t const*)str.data(), str.size());
}

using StringList = std::vector<String>;

enum class DeviceType { Undefined, Harddisk, Floppy, CdRom };

struct Partition {
    String ntName;
    String link;
    String name;
    char mountDrive = 0;
};
using PartitionMap = std::unordered_map<uint32_t, Partition>;

struct DeviceInfo {
    String name;
    DeviceType type = DeviceType::Undefined;
    PartitionMap partitions;
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
    DeviceType type = DeviceType::Undefined;
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
}

#endif // DISKTYPES_H
