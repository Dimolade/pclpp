#include "pclpp_stringhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

void CreateString(PCLPP_MemoryReference& mr, std::string string, PCLPP* pclpp)
{
    PCLPP_Block& b = pclpp->blocks.back();
    b.assembly.MOVRImm(0, mr.index);
    b.assembly.MOVRImm(1, mr.partofthis);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
    b.assembly.CallFunction((uint32_t)pclpp_std::Free); // free the address of variable
    b.assembly.MOVRImm(0, string.length()+1); // +1 for null terminator
    b.assembly.MOVRImm(1, 1); // (sizeof char)
    b.assembly.CallFunction((uint32_t)pclpp_std::Calloc); // r0: address
    b.assembly.MOVRR(6,0);
    b.assembly.PUSH(1 << 6);
    // Create Characters
    int i = 0;
    for (i = 0; i < string.length(); i++)
    {
        b.assembly.MOVRImm(1, (uint8_t)string[i]); // mov char
        b.assembly.PUSH(1 << 0);
        b.assembly.CallFunction((uint32_t)pclpp_std::Write8);
        b.assembly.POP(1 << 0);
        b.assembly.ADDRImm(0, 1); // add address
    }
    b.assembly.MOVRImm(1, '\0');
    b.assembly.CallFunction((uint32_t)pclpp_std::Write8); // add null terminator
    // allocate 4 bytes for address (address to address)
    b.assembly.MOVRImm(0, 4); // size
    b.assembly.CallFunction((uint32_t)pclpp_std::Malloc);
    b.assembly.MOVRR(7, 0);
    b.assembly.POP(1 << 6);
    b.assembly.MOVRR(1, 6);
    b.assembly.CallFunction((uint32_t)pclpp_std::Write32); // set address
    b.assembly.MOVRR(0, 7);
    b.assembly.MOVRImm(1, mr.index);
    b.assembly.CallFunction((uint32_t)pclpp_std::AllocateLocal);

    mr.size = 4; // is now a byte 4
}

void PCLPP_StringHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "string") return;
    if (PCLPP->inBlock == false) return;

    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_MemoryReference& mr = PCLPP->GetReference(name);

    std::string string = PCLPP->tokenizer.tokens.Advance();

    mr.doublefree = true;

    CreateString(mr, string, PCLPP);

    PCLPP->tokenizer.tokens.Advance(); // skip ;
}