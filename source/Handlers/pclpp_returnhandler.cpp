#include "pclpp_returnhandler.h"
#include "../plcpp_compiler.hpp"

PCLPP_MemoryReference& GetReferenceRecursive(PCLPP_MemoryReference& mrr, std::string token, PCLPP* pclpp)
{
    PCLPP_MemoryReference* cand = nullptr;

    for (PCLPP_MemoryReference& mr : mrr.children)
    {
        if (mr.name == token)
        {
            cand = &mr;
            break;
        }
    }

    std::string next = pclpp->tokenizer.tokens.Advance();

    if (next == ".")
    {
        return GetReferenceRecursive(*cand, pclpp->tokenizer.tokens.Advance(), pclpp);
    }

    pclpp->tokenizer.tokens.iteration--;
    return *cand;
}

PCLPP_MemoryReference& GetReference(PCLPP* pclpp, std::string token)
{
    PCLPP_Block& thisBlock = pclpp->blocks.back();
    PCLPP_MemoryReference* cand;
    for (PCLPP_MemoryReference& mr : thisBlock.memoryReferences)
    {
        if (mr.name == token)
        {
            cand = &mr;
            break;
        }
    }
    std::string next = pclpp->tokenizer.tokens.Advance();
    if (next == ".")
    {
        return GetReferenceRecursive(*cand, pclpp->tokenizer.tokens.Advance(), pclpp);
    }
    pclpp->tokenizer.tokens.iteration--;
    return *cand;
}

void InputPointerClass(PCLPP* pclpp, const std::string& token)
{
    std::string other = pclpp->tokenizer.tokens.Advance();
    if (isdigit(other[0]))
    {
        pclpp->blocks.back().assembly.MOVRImm(0, stoi(other));
    }
    else
    {
        PCLPP_MemoryReference& mr = GetReference(pclpp, other);
        pclpp->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::GetLocal(mr.index));
        std::string next = pclpp->tokenizer.tokens.Advance(); // either ; or *
        if (next == ";")
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