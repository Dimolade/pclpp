#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "end" && PCLPP->inBlock)
    {
        
        return;
    }
}