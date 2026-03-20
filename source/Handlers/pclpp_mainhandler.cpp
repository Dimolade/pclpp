#include "pclpp_mainhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "}" && PCLPP->inBlock)
    {
        PCLPP->UnallocateBlock(PCLPP->blocks.back());
        PCLPP->blocks.back().assembly.POP_LR();
        PCLPP->blocks.back().assembly.BXLR();
        return;
    }
    if (token != "main") return;
    std::string next = PCLPP->tokenizer.tokens.Advance();
    if (next != "{") return;
    PCLPP->inBlock = true;
    PCLPP_Block& b = PCLPP->blocks.emplace_back();
    b.type = PCLPP_Block_Type::Main;
    b.assembly.PUSH_LR();
    b.assembly.MOVRImm(0, -1);
}