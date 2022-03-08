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

struct Partition {
    String ntName;
    String link;
    String name;
    char mountDrive;
};
using PartitionMap = std::unordered_map<uint32_t, Partition>;

struct DeviceInfo {
    String name;
    PartitionMap partitions;
};
using DeviceInfoList = std::vector<DeviceInfo>;
}

#endif // DISKTYPES_H
