#include "pclpp_edithandler.h"
#include "../plcpp_compiler.hpp"

class IndexHolder
{
public:
    uint16_t index;
    uint8_t reg;
};

void LoadVar(uint8_t targetRegister, PCLPP_MemoryReference& mr, PCLPP_Block& b, PCLPP* pclpp)
{
    b.assembly.PUSH(1 << 0);
    b.assembly.PUSH(1 << 1);
    b.assembly.MOVRImm(0, mr.index);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
    b.assembly.MOVRR(targetRegister, 0);
    pclpp->ReadASM(mr.size);
    if (targetRegister != 0)
        b.assembly.POP(1 << 0);

    b.assembly.POP(1 << 1);
}

void ReloadVar(IndexHolder& ih, PCLPP_MemoryReference& mr, PCLPP_Block& b, PCLPP* pclpp)
{
    b.assembly.PUSH(1 << 0);
    b.assembly.PUSH(1 << 1);
    
    b.assembly.MOVRR(0, ih.index);
    b.assembly.MOVRR(1, 0);
    b.assembly.PUSH(1 << 1);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal); // r0: address, r1: current value of register
    b.assembly.POP(1 << 1);
    pclpp->WriteASM(mr.size, b); // write value
    
    b.assembly.POP(1 << 0);
    b.assembly.POP(1 << 1);
}

IndexHolder& Get(uint16_t index, std::vector<IndexHolder> hs)
{
    for (IndexHolder& h : hs)
    {
        if (h.index == index) return h;
    }
    return;
}

void PCLPP_EditHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "edit") return;
    if (PCLPP->inBlock == false) return;
    PCLPP_Block& b = PCLPP->blocks.back();

    std::vector<PCLPP_MemoryReference> mrrs;
    std::vector<IndexHolder> holders;

    PCLPP->tokenizer.tokens.Advance(); // skip "("
    std::string now = PCLPP->tokenizer.tokens.Advance();
    while (now != ")")
    {
        if (now == ",")
        {
            now = PCLPP->tokenizer.tokens.Advance();
            continue;
        }
        mrrs.push_back(PCLPP->GetReference(now));
        now = PCLPP->tokenizer.tokens.Advance();
    }
    PCLPP->tokenizer.tokens.Advance(); // skip ")"
    PCLPP->tokenizer.tokens.Advance(); // skip "{"
    for (int i = 0; i < mrrs.size(); i++)
    {
        IndexHolder& ih = holders.emplace_back();
        ih.index = mrrs[i].index;
        ih.reg = i;
        LoadVar(i, mrrs[i], b, PCLPP); // load all vars into their registers
    }
    while (now != "}")
    {
        PCLPP_MemoryReference& mr = PCLPP->GetReference(now);
        IndexHolder& ih = Get(mr.index, holders);
        now = PCLPP->tokenizer.tokens.Advance();
        if (now == "=")
        {
            now = PCLPP->tokenizer.tokens.Advance();
            if (isdigit(now[0]))
            {
                b.assembly.MOVRImm(ih.reg, stoi(now));
            }
            else
            {
                PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
                IndexHolder& ih2 = Get(mr.index, holders);
                b.assembly.MOVRR(ih.reg, ih2.reg);
            }
        }
        else if (now == "exchange") // exchange addresses of 2 References (i hope)
        {
            PCLPP_MemoryReference& mr = PCLPP->GetReference(now);
            IndexHolder& ih = Get(mr.index, holders);
            now = PCLPP->tokenizer.tokens.Advance();
            PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
            IndexHolder& ih2 = Get(mr.index, holders);

            uint16_t a = mr.index;
            uint16_t b = mr2.index;
            mr.index = b;
            mr2.index = a;
        }
        else if (now == "+")
        {
            PCLPP->tokenizer.tokens.Advance(); // skip =
            now = PCLPP->tokenizer.tokens.Advance();
            if (isdigit(now[0]))
            {
                b.assembly.ADDRImm(ih.reg, stoi(now));
            }
            else
            {
                PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
                IndexHolder& ih2 = Get(mr.index, holders);
                b.assembly.ADDRR(ih.reg, ih2.reg, ih.reg);
            }
        }

        PCLPP->tokenizer.tokens.Advance(); // skip semicolon
    }

    // Restore Variables
    for (int i = 0; i < mrrs.size(); i++)
    {
        IndexHolder& ih = holders[i];
        ReloadVar(ih, mrrs[i], b, PCLPP);
    }
}