#include "pclpp_mainhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "}" && PCLPP->inBlock)
    {
        PCLPP->blocks.back().assembly.PUSH(1 << 0);
        PCLPP->UnallocateBlock(PCLPP->blocks.back());
        PCLPP->blocks.back().assembly.MOVRImm(0,0);
        PCLPP->blocks.back().assembly.POP(1 << 0);
        PCLPP->blocks.back().assembly.POP(1 << 11);
        PCLPP->blocks.back().assembly.MOVRR(14, 11);
        PCLPP->blocks.back().assembly.BXLR();
        PCLPP->inBlock = false;
        return;
    }
    if (token != "main") return;
    std::string next = PCLPP->tokenizer.tokens.Advance();
    if (next != "{") return;
    PCLPP->inBlock = true;
    PCLPP_Block& b = PCLPP->blocks.emplace_back();
    b.type = PCLPP_Block_Type::Main;
    PCLPP->blocks.back().assembly.MOVRR(11, 14);
    PCLPP->blocks.back().assembly.PUSH(1 << 11);
    b.assembly.MOVRImm(0, -1);
}