#include "pclpp_classhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_ClassHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "class") return;
}