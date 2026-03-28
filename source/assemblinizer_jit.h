#pragma once
#define kynex_CTRL

#include "assemblinizer.hpp"

typedef int(*Fn_t)(uint32_t);

struct PCLPP_Assembly_Runner_Result
{
    bool success;
    std::string failReason;

    PCLPP_Assembly_Runner_Result(bool s, std::string fr = "") :
    success(s), failReason(fr) {}
};

struct PCLPP_Assembly_Runner
{
    inline static PCLPP_Assembly_Runner_Result Load(PCLPP_Assembly& ASM, uint32_t index = 0)
    {
        if (ASM.startAddress == 0)
        {
            if (!ASM.allocStartAddress())
            {
                return PCLPP_Assembly_Runner_Result(false, "Failed allocating code block.");
            }
            if (!ASM.setupInstructions())
            {
                return PCLPP_Assembly_Runner_Result(false, "Failed copying instructions.");
            }
            if (!ASM.commitCodeRegion())
            {
                return PCLPP_Assembly_Runner_Result(false, "Failed commiting code region.");
            }
            if (!ASM.allocStartAddress_AfterCommit(index))
            {
                return PCLPP_Assembly_Runner_Result(false, "Failed retrieving address of function.");
            }
        }
        return PCLPP_Assembly_Runner_Result(true);
    }
    inline static int Run(PCLPP_Assembly& ASM, uint32_t index = 0, uint32_t input = 0)
    {
        if (ASM.startAddress == 0)
        {
            return -1;
        }
        Fn_t t = (Fn_t)ASM.startAddress;
        int out = t(input);
        return out;
    }

    inline static void Unallocate(PCLPP_Assembly ASM)
    {
        if (ASM.startAddress == 0) return;
        ASM.unalloc();
    }
};
