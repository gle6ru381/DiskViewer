#include "diskviewer.h"
#include "fsoperations.h"
#include <iostream>

using namespace dv;

DeviceInfoList dv::physicalDevices()
{
    DeviceInfoList result;
    auto volumes = getVolumes();
    auto ntDevices = getNtBlockDevices();

#ifdef DEVICE_VIEWER_DEBUG_OUTPUT
    std::wcerr << "Nt devices:\n";
    for (auto const& device : ntDevices) {
        std::wcerr << (wchar_t*)device.name.c_str() << L' '
                   << (wchar_t*)device.link.c_str() << '\n';
        std::wcerr << "Partitions:\n";
        for (auto const& part : device.partitions) {
            std::wcerr << (wchar_t*)part.name.c_str() << L' '
                       << (wchar_t*)part.link.c_str() << L'\n';
        }
        std::wcerr << L'\n';
    }

    std::wcerr << "Volumes:\n";
    for (auto const& volume : volumes) {
        std::wcerr << (wchar_t*)volume.name.c_str() << L' '
                   << (wchar_t*)volume.link.c_str() << L' '
                   << (volume.mountDrive == '\0' ? ' ' : volume.mountDrive)
                   << L'\n';
    }
#endif

    for (auto& device : ntDevices) {
        DeviceInfo info;
        for (auto part = device.partitions.begin();
             part != device.partitions.end();) {
            bool isFind = false;
            for (auto volume = volumes.begin(); volume != volumes.end();) {
                if (volume->link == part->link) {
                    Partition p;
                    p.link = std::move(volume->link);
                    p.ntName = std::move(volume->name);
                    p.mountDrive = volume->mountDrive;
                    uint32_t number;
                    swscanf_s(
                            (wchar_t*)part->name.c_str(),
                            L"Partition%u",
                            &number);
                    p.name = std::move(part->name);
                    info.partitions.emplace(number, p);
                    volume = volumes.erase(volume);
                    part = device.partitions.erase(part);
                    isFind = true;
                    break;
                } else
                    volume++;
            }
            if (!isFind) {
                Partition p;
                uint32_t number;
                swscanf_s(
                        (wchar_t*)part->name.c_str(), L"Partition%u", &number);
                p.link = std::move(part->link);
                p.mountDrive = '\0';
                p.name = std::move(part->name);
                info.partitions.emplace(number, p);
                part = device.partitions.erase(part);
            }
        }
        info.name = std::move(device.name);
        info.type = device.type;
        result.emplace_back(info);
    }

    return result;
}

uint64_t DeviceInfo::size() const
{
    std::cerr << "Enter function\n";
    auto name = partitions.at(0).link; // this->name + u"\\Partition3";
    std::wcerr << "Size file: " << toWide(name) << '\n';
    auto h = dv::nativeCreateFile(name, 0, 0); // openFile(name, GENERIC_READ);
    if (h == INVALID_HANDLE_VALUE) {
        auto err = GetLastError();
        auto format = formatError(err);
        std::cerr << "Open error: " << format << '\n';
        return 0;
    }
    auto size = getDiskSize(h);
    dv::closeFile(h);
    return size;
}
