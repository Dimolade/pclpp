#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "end" && PCLPP->inBlock)
    {
        std::vector<uint8_t> codeSection;
        PCLPP_Block& b = PCLPP->blocks.back();
        PCLPP_SubBlockPoint& sbp = b.subPoints.back();
        for (int i = sbp.codePoint+1; i < b.assembly.code.size(); i++)
        {
            codeSection.push_back(b.assembly.code[i]);
        }
        b.assembly.code.erase(sbp.codePoint+1,b.assembly.code.size());
        b.assembly.MOVRImm(0, codeSection.size()-4);
        b.assembly.ADDRR(15,0,15,2); // ADDNE r15, r0, r15 ; this skips the instructions if the if condition isnt met
        b.assembly.code.reserve(b.assembly.code.size()+codeSection.size());
        b.assembly.code.insert(b.assembly.code.end(), codeSection.begin(), codeSection.end());
        b.subPoints.pop_back();
        return;
    }
}