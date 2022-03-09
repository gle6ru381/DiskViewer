#ifndef NATIVEFUNC_H
#define NATIVEFUNC_H

#include "nativetypes.h"

namespace dv {

class NativeFunc {
public:
    static NativeFunc const* instance()
    {
        if (!m_instance)
            m_instance = new NativeFunc;
        return m_instance;
    }

    nt::NtOpenSymbolicLinkObjectT NtOpenSymbolicLinkObject;
    nt::NtQuerySymbolicLinkObjectT NtQuerySymbolicLinkObject;
    nt::NtOpenDirectoryObjectT NtOpenDirectoryObject;
    nt::NtQueryDirectoryObjectT NtQueryDirectoryObject;
    nt::RtlNtStatusToDosErrorT RtlNtStatusToDosError;
    nt::RtlInitUnicodeStringT RtlInitUnicodeString;
    nt::NtOpenFileT NtOpenFile;

private:
    static NativeFunc* m_instance;

    NativeFunc();
};

}

#endif // NATIVEFUNC_H
