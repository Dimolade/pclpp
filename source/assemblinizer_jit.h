
#define kynex_CTRL

#include "assemblinizer.hpp"

typedef int(*Fn_t)(int);

struct Assemblinizer
{
    inline static int Run(Assembly ASM, int input = 0)
    {
        if (ASM.startAddress == 0)
        {
            ASM.allocStartAddress();
            ASM.setupInstructions();
            ASM.commitCodeRegion();
            ASM.allocStartAddress_AfterCommit();
        }
        Fn_t t = (Fn_t)ASM.startAddress;
        int out = t(input);
        return out;
    }
};
