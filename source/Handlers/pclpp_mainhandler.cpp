#include "pclpp_mainhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "end")
    {
        PCLPP->assembly.POP_LR();
        PCLPP->assembly.BXLR();
    }
    if (token != "main") return;
    PCLPP->assembly.PUSH_LR();
    PCLPP->assembly.MOVRImm(0, 71);
}