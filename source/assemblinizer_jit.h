#pragma once
#define kynex_CTRL

#include "assemblinizer.hpp"

typedef int(*Fn_t)(uint32_t);

struct Assemblinizer
{
    inline static Fn_t Get(Assembly& ASM, uint32_t index = 0)
    {
        if (ASM.startAddress == 0)
        {
            ASM.allocStartAddress();
            ASM.setupInstructions();
            ASM.commitCodeRegion();
            ASM.allocStartAddress_AfterCommit(index);
        }
        return (Fn_t)ASM.startAddress;
    }
    inline static int Run(Assembly& ASM, uint32_t index = 0, uint32_t input = 0)
    {
        if (ASM.startAddress == 0)
        {
            ASM.allocStartAddress();
            ASM.setupInstructions();
            ASM.commitCodeRegion();
            ASM.allocStartAddress_AfterCommit(index);
        }
        Fn_t t = (Fn_t)ASM.startAddress;
        int out = t(input);
        return out;
    }

    inline static void Unallocate(Assembly ASM)
    {
        if (ASM.startAddress == 0) return;
        ASM.unalloc();
    }
};
