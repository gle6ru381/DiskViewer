#ifndef NTDIR_H
#define NTDIR_H

#include "disktypes.h"
#include "nativetypes.h"

namespace dv {

class NtDir {
public:
    NtDir() = default;
    NtDir(StringView dName) : m_name(dName)
    {
    }
    void setName(StringView name);

    bool open();
    bool read(nt::POBJDIR_INFORMATION info, ULONG infoSize);
    void close();

    ~NtDir()
    {
        close();
    }

private:
    String m_name;
    HANDLE h = INVALID_HANDLE_VALUE;
    ULONG context = 0;
};

}
#endif // NTDIR_H
