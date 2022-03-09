#include "ntdir.h"
#include "nativefunc.h"

using namespace dv;

void NtDir::setName(StringView name)
{
    close();
    m_name = name;
}

bool NtDir::open()
{
    using OBJECT_ATTRIBUTES = nt::OBJECT_ATTRIBUTES;
    if (m_name.empty())
        return false;

    auto f = NativeFunc::instance();

    nt::UNICODE_STRING uname;
    OBJECT_ATTRIBUTES objAttrs;

    f->RtlInitUnicodeString(&uname, (wchar_t*)m_name.c_str());
    InitializeObjectAttributes(
            &objAttrs, &uname, OBJ_CASE_INSENSITIVE, 0, nullptr);

    auto status = f->NtOpenDirectoryObject(
            &h, STANDARD_RIGHTS_READ | 0x0001, &objAttrs);

    if (!NT_SUCCESS(status))
        h = INVALID_HANDLE_VALUE;

    return h != INVALID_HANDLE_VALUE;
}

bool NtDir::read(nt::POBJDIR_INFORMATION info, ULONG infoSize)
{
    if (h == INVALID_HANDLE_VALUE)
        return false;

    auto f = NativeFunc::instance();

    auto status = f->NtQueryDirectoryObject(
            h, info, infoSize, true, false, &context, nullptr);

    return NT_SUCCESS(status);
}

void NtDir::close()
{
    if (h == INVALID_HANDLE_VALUE)
        return;
    CloseHandle(h);
    h = INVALID_HANDLE_VALUE;
    context = 0;
}
