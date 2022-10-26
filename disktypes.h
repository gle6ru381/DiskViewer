#ifndef DISKTYPES_H
#define DISKTYPES_H

#include "DiskViewer_global.h"
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace dv {
using String = std::u16string;
using StringView = std::u16string_view;

inline std::wstring_view toWide(String const& str)
{
    return std::wstring_view((wchar_t const*)str.data(), str.size());
}

inline std::wstring toWide(String&& str)
{
    return std::wstring(str.begin(), str.end());
}

inline std::wstring_view toWide(StringView str)
{
    return std::wstring_view((wchar_t const*)str.data(), str.size());
}

using StringList = std::vector<String>;

enum class DeviceType { Undefined, Harddisk, Floppy, CdRom };

inline String deviceTypeToString(DeviceType type)
{
    switch (type) {
    case DeviceType::Undefined:
        return u"Undefined";
    case DeviceType::Harddisk:
        return u"Harddisk";
    case DeviceType::Floppy:
        return u"Floppy";
    case DeviceType::CdRom:
        return u"CdRom";
    }
}

struct Partition {
    String ntName;
    String link;
    String name;
    char mountDrive = 0;
    operator String() const
    {
        return (ntName.empty() ? u"No nt name" : u"NtName: " + ntName)
                + (link.empty() ? u"" : u", link by: " + link) + u", name: "
                + name
                + (mountDrive ? u", mount on: " + String(1, mountDrive) : u"");
    }
};
using PartitionMap = std::unordered_map<uint32_t, Partition>;

struct DISKVIEWER_EXPORT DeviceInfo {
    String name;
    DeviceType type = DeviceType::Undefined;
    PartitionMap partitions;

    uint64_t size() const;

    operator String() const
    {
        auto s = std::to_string(size());
        auto result = u"Name: " + name + u", type: " + deviceTypeToString(type)
                + u", size: " + String(s.begin(), s.end())
                + +u", partitions:\n";
        for (auto const& part : partitions) {
            auto p = std::to_string(part.first);
            result += u"Partition: " + String(p.begin(), p.end()) + u", ";
            result += part.second;
            result += u"\n";
        }

        return result;
    }
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
