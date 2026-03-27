#include "pclpp_stringhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

void CreateString(PCLPP_MemoryReference& mr, std::string string, PCLPP* pclpp)
{
    PCLPP_Block& b = pclpp->blocks.back();
    b.assembly.MOVRImm(0, mr.index);
    b.assembly.MOVRImm(1, mr.partofthis);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
    b.assembly.CallFunction((uint32_t)pclpp_std::Free); // free variable
    b.assembly.MOVRImm(0, 0);
}

void PCLPP_StringHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "string") return;
    if (PCLPP->inBlock == false) return;

    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_MemoryReference& mr = PCLPP->GetReference(name);

    std::string string = PCLPP->tokenizer.tokens.Advance();

    std::cout << "Creating String " << string << std::endl;

    //CreateString(mr, string, PCLPP):

    PCLPP->tokenizer.tokens.Advance(); // skip ;
}