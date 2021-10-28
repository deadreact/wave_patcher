#include "littleendianint.h"

bool isHostLittleEndian()
{
    static EHostEndianness hostEndianness = EHostEndianness::Undefined;

    if (hostEndianness == EHostEndianness::Undefined)
    {
        int i = 1;
        char *p = (char *)&i;

        hostEndianness = (p[0] == 1) ? EHostEndianness::LittleEndian : EHostEndianness::BigEndian;
    }
    return hostEndianness == EHostEndianness::LittleEndian;
}
