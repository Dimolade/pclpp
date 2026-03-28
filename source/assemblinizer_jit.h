#pragma once
#define kynex_CTRL

#include "assemblinizer.hpp"

typedef int(*Fn_t)(uint32_t);

struct PCLPP_Assembly_Runner_Result
{
    bool success;
    std::string failReason;

    PCLPP_Assembly_Runner_Result(bool s, std::string fr = "")
    success(s), failReason(fr) {}
};

struct PCLPP_Assembly_Runner
{
    inline static PCLPP_Assembly_Runner_Result Load(Assembly& ASM, uint32_t index = 0)
    {
        if (ASM.startAddress == 0)
        {
            if (!ASM.allocStartAddress()) return false;
            if (!ASM.setupInstructions()) return false;
            if (!ASM.commitCodeRegion()) return false;
            if (!ASM.allocStartAddress_AfterCommit(index)) return false;
        }
        return PCLPP_Assembly_Runner_Result(true);
    }
    inline static int Run(Assembly& ASM, uint32_t index = 0, uint32_t input = 0)
    {
        if (ASM.startAddress == 0)
        {
            return -1;
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
