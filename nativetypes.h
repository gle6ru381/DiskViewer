#ifndef NATIVETYPES_H
#define NATIVETYPES_H
#include <windows.h>

namespace dv { namespace nt {
#include <ntdef.h>

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

using NtOpenSymbolicLinkObjectT
        = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
using NtQuerySymbolicLinkObjectT
        = NTSTATUS (*)(HANDLE, PUNICODE_STRING, PULONG);
using NtOpenDirectoryObjectT
        = NTSTATUS (*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
using NtQueryDirectoryObjectT = NTSTATUS (*)(
        HANDLE, POBJDIR_INFORMATION, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
using RtlNtStatusToDosErrorT = DWORD (*)(NTSTATUS);
using RtlInitUnicodeStringT = NTSTATUS (*)(PUNICODE_STRING, PWCHAR);
using NtOpenFileT = NTSTATUS (*)(
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        ULONG,
        ULONG);
}}

#endif // NATIVETYPES_H
