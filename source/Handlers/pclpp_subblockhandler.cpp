#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

#define mainhandler_debug

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "end" && PCLPP->inBlock)
    {
        
        return;
    }
}