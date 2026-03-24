#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

#define mainhandler_debug

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "}" && PCLPP->inBlock)
    {
        PCLPP->blocks.back().assembly.MOVRR(12, 0);
        PCLPP->blocks.back().assembly.PUSH(1 << 12);
        PCLPP->UnallocateBlock(PCLPP->blocks.back());
        PCLPP->blocks.back().assembly.POP(1 << 12);
        PCLPP->blocks.back().assembly.MOVRR(0, 12);
        PCLPP->blocks.back().assembly.POP(1 << 11);
        PCLPP->blocks.back().assembly.MOVRR(14, 11);
        PCLPP->blocks.back().assembly.BXLR();
        PCLPP->inBlock = false;
        if (PCLPP->blocks.back().type == PCLPP_Block_Type::Function)
        {
            if (PCLPP->allowAutoBlockInitialization)
            {
                #ifdef mainhandler_debug
                std::cout << "Instruction Count: " << std::to_string(PCLPP->blocks.back().assembly.instructs) << std::endl;
                std::cout << "Trying to allocate for Class Function." << std::endl;
                #endif
                Assemblinizer::Get(PCLPP->blocks.back().assembly, PCLPP->codepageamount); // init to get the startAddress
                PCLPP->codepageamount++;
                #ifdef mainhandler_debug
                std::cout << "Attempted Start Address: " << std::to_string(PCLPP->blocks.back().assembly.startAddress) << std::endl;
                #endif
            }
        }
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