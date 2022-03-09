#include "nativefunc.h"

using namespace dv;

NativeFunc* NativeFunc::m_instance = nullptr;

NativeFunc::NativeFunc()
{
    auto lib = LoadLibraryW((wchar_t const*)u"Ntdll.dll");
    NtOpenSymbolicLinkObject = (nt::NtOpenSymbolicLinkObjectT)GetProcAddress(
            lib, "NtOpenSymbolicLinkObject");
    NtQuerySymbolicLinkObject = (nt::NtQuerySymbolicLinkObjectT)GetProcAddress(
            lib, "NtQuerySymbolicLinkObject");
    NtOpenDirectoryObject = (nt::NtOpenDirectoryObjectT)GetProcAddress(
            lib, "NtOpenDirectoryObject");
    NtQueryDirectoryObject = (nt::NtQueryDirectoryObjectT)GetProcAddress(
            lib, "NtQueryDirectoryObject");
    RtlNtStatusToDosError = (nt::RtlNtStatusToDosErrorT)GetProcAddress(
            lib, "RtlNtStatusToDosError");
    RtlInitUnicodeString = (nt::RtlInitUnicodeStringT)GetProcAddress(
            lib, "RtlInitUnicodeString");
    NtOpenFile = (nt::NtOpenFileT)GetProcAddress(lib, "NtOpenFile");
}
