#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

void IfHandler(PCLPP* PCLPP, const std::string& token)
{
    PCLPP_Block& b = PCLPP->blocks.back();
    PCLPP->tokenizer.tokens.Advance(); // skip paranthesis

    std::string leftSideMRName = PCLPP->tokenizer.tokens.Advance();
    bool leftPointer = false;
    std::string operand;
    leftP:
    operand = PCLPP->tokenizer.tokens.Advance();
    if (operand == "*")
    {
        leftPointer = true;
        goto leftP;
    }
    std::string rightSideMRName = PCLPP->tokenizer.tokens.Advance();
    bool rightPointer = false;
    std::string pp = PCLPP->tokenizer.tokens.Advance(); // skip paranthesis
    if (pp == "*")
    {
        rightPointer = true;
        PCLPP->tokenizer.tokens.Advance(); // skip paranthesis
    }

    PCLPP_MemoryReference& left = PCLPP->GetReference(leftSideMRName);
    PCLPP_MemoryReference& right = PCLPP->GetReference(rightSideMRName);

    uint8_t startReg = 5;
    for (uint8_t i = 0; i < 2; i++)
    {
        PCLPP_MemoryReference& now = i == 0 ? left : right;
        b.assembly.MOVRImm(0, now.index);
        b.assembly.MOVRImm(1, now.partofthis);
        b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal); // r0: address
        if (!(i == 0 ? leftPointer : rightPointer))
        {
            PCLPP->ReadASM(now.size, b);
        }
        b.assembly.MOVRR(0, startReg+i); // r0: value or address
    }

    if (operand == "==") // funny
    {
        b.assembly.CMPRR(5,6); // boom
    }

    PCLPP_SubBlockPoint& sbp = b.subPoints.emplace_back();
    sbp.codePoint = b.assembly.code.size();
}

void PCLPP_SubBlockHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (PCLPP->inBlock) return;
    if (token == "end")
    {
        std::vector<uint8_t> codeSection;
        PCLPP_Block& b = PCLPP->blocks.back();
        PCLPP_SubBlockPoint& sbp = b.subPoints.back();
        for (int i = sbp.codePoint; i < b.assembly.code.size(); i++)
        {
            codeSection.push_back(b.assembly.code[i]);
        }
        b.assembly.code.erase(b.assembly.code.begin()+sbp.codePoint,b.assembly.code.end());
        b.assembly.MOVRImm(0, codeSection.size()-4);
        b.assembly.ADDRR(15,0,15,2); // ADDNE r15, r0, r15 ; this skips the instructions if the if condition isnt met
        b.assembly.code.reserve(b.assembly.code.size()+codeSection.size());
        b.assembly.code.insert(b.assembly.code.end(), codeSection.begin(), codeSection.end());
        b.subPoints.pop_back();
        return;
    }

    if (token == "if")
    {
        IfHandler(PCLPP, token);
    }
}