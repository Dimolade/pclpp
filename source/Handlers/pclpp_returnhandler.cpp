#include "pclpp_returnhandler.h"
#include "../plcpp_compiler.hpp"

void InputPointerClass(PCLPP* pclpp, const std::string& token)
{
    std::string other = pclpp->tokenizer.tokens.Advance();
    if (isdigit(other[0]))
    {
        pclpp->blocks.back().assembly.MOVRImm(0, stoi(other));
    }
    else
    {
        PCLPP_MemoryReference& mr = pclpp->GetReference(other);
        pclpp->blocks.back().assembly.MOVRImm(0, mr.index);
        pclpp->blocks.back().assembly.MOVRImm(1, mr.partofthis);
        pclpp->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
        std::string next = pclpp->tokenizer.tokens.Advance(); // either ; or *
        if (next == ";")
        {
            switch (mr.size)
            {
                case 1:
                pclpp->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read8);
                break;
                case 2:
                pclpp->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read16);
                break;
                case 4:
                pclpp->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read32);
                break;
            }
        }
        else if (next == "*")
        {
            pclpp->tokenizer.tokens.Advance(); // go to ;
        }
    }
}

void PCLPP_ReturnHandler::OnToken(PCLPP* pclpp, const std::string& token)
{
    if (token == "returnset")
    {
        InputPointerClass(pclpp, token);
    }
    else if (token == "return")
    {
        InputPointerClass(pclpp, token);
        pclpp->UnallocateBlock(pclpp->blocks.back());
        pclpp->blocks.back().assembly.POP_LR();
        pclpp->blocks.back().assembly.BXLR();
    }
}