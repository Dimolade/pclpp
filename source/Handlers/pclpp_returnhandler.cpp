#include "pclpp_returnhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_ReturnHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "returnset")
    {
        std::string other = PCLPP->tokenizer.tokens.Advance();
        PCLPP->.assembly.MOVRImm(0, stoi(other));
        std::string final = PCLPP->tokenizer.tokens.Advance();
    }
}