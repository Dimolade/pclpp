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
    b.assembly.PUSH(1 << 1);
    b.assembly.PUSH(1 << 0);
    b.assembly.MOVRImm(0, mr.index); // r0: index
    b.assembly.PUSH(1 << 1);
    b.assembly.MOVRImm(1, mr.partofthis);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal); // r0: address
    b.assembly.POP(1 << 1);
    pclpp->ReadASM(mr.size, b); // r0: value
    b.assembly.MOVRR(targetRegister, 0);
    if (targetRegister == 0)
    {
        b.assembly.MOVRR(1,0);
        b.assembly.POP(1 << 0);
        b.assembly.MOVRR(0,1);
    }
    else
        b.assembly.POP(1 << 0);

    if (targetRegister == 1)
    {
        b.assembly.PUSH(1 << 9);
        b.assembly.MOVRR(9, 1);
        b.assembly.POP(1 << 1);
        b.assembly.MOVRR(1,9);
        b.assembly.POP(1 << 9);
    }
    else
        b.assembly.POP(1 << 1);
}

void ReloadVar(IndexHolder& ih, PCLPP_MemoryReference& mr, PCLPP_Block& b, PCLPP* pclpp)
{
    b.assembly.PUSH(1 << 0);
    b.assembly.PUSH(1 << 1);
    
    b.assembly.MOVRR(1, ih.reg); // r1: value
    b.assembly.MOVRImm(0, ih.index); // r0: index
    b.assembly.PUSH(1 << 1);
    b.assembly.MOVRImm(1, mr.partofthis);
    b.assembly.CallFunction((uint32_t)pclpp_std::GetLocal); // r0: address
    b.assembly.POP(1 << 1); // r1: value
    pclpp->WriteASM(mr.size, b); // write value to address

    b.assembly.POP(1 << 1);
    b.assembly.POP(1 << 0);
}

void DivideImm(IndexHolder& ih, PCLPP_Block& b, uint32_t value)
{
    b.assembly.PUSH(1 << 1);

    if (ih.reg != 0)
        b.assembly.PUSH(1 << 0);

    b.assembly.MOVRR(0, ih.reg);
    b.assembly.MOVRImm(1, value);

    b.assembly.CallFunction((uint32_t)pclpp_std::Divide);

    if (ih.reg != 0)
        b.assembly.MOVRR(ih.reg, 0);

    if (ih.reg != 0)
        b.assembly.POP(1 << 0);

    b.assembly.POP(1 << 1);
}

void DivideRR(IndexHolder& ih, PCLPP_Block& b, uint8_t reg2)
{
    b.assembly.PUSH(1 << 1);

    if (ih.reg != 0)
        b.assembly.PUSH(1 << 0);

    b.assembly.MOVRR(0, ih.reg);
    b.assembly.MOVRR(1, reg2);

    b.assembly.CallFunction((uint32_t)pclpp_std::Divide);

    if (ih.reg != 0)
        b.assembly.MOVRR(ih.reg, 0);

    if (ih.reg != 0)
        b.assembly.POP(1 << 0);

    b.assembly.POP(1 << 1);
}

IndexHolder& Get(uint16_t index, std::vector<IndexHolder>& hs)
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
    now = PCLPP->tokenizer.tokens.data[PCLPP->tokenizer.tokens.iteration-1];
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
                IndexHolder& ih2 = Get(mr2.index, holders);
                b.assembly.MOVRR(ih.reg, ih2.reg);
            }
        }
        else if (now == "exchange") // exchange addresses of 2 References (i hope)
        {
            now = PCLPP->tokenizer.tokens.Advance();
            PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
            IndexHolder& ih2 = Get(mr2.index, holders);

            uint16_t a = mr.index;
            uint16_t b = mr2.index;
            uint8_t partofthisa = mr.partofthis;
            uint8_t partofthisb = mr2.partofthis;
            mr.index = b;
            mr.partofthis = partofthisb;
            mr2.index = a;
            mr2.partofthis = partofthisa;
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
                IndexHolder& ih2 = Get(mr2.index, holders);
                b.assembly.ADDRR(ih.reg, ih2.reg, ih.reg);
            }
        }
        else if (now == "-")
        {
            PCLPP->tokenizer.tokens.Advance(); // skip =
            now = PCLPP->tokenizer.tokens.Advance();
            if (isdigit(now[0]))
            {
                b.assembly.SUBRImm(ih.reg, stoi(now));
            }
            else
            {
                PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
                IndexHolder& ih2 = Get(mr2.index, holders);
                b.assembly.SUBRR(ih.reg, ih2.reg, ih.reg);
            }
        }
        else if (now == "*")
        {
            PCLPP->tokenizer.tokens.Advance(); // skip =
            now = PCLPP->tokenizer.tokens.Advance();
            if (isdigit(now[0]))
            {
                uint8_t storereg = 9;
                if (ih.reg == storereg)
                {
                    storereg = 8;
                }
                b.assembly.PUSH(1 << storereg);
                b.assembly.MOVRImm(storereg, stoi(now));
                b.assembly.MULRR(ih.reg, storereg);
                b.assembly.POP(1 << storereg);
            }
            else
            {
                PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
                IndexHolder& ih2 = Get(mr2.index, holders);
                b.assembly.MULRR(ih.reg, ih2.reg, ih.reg);
            }
        }
        else if (now == "/")
        {
            PCLPP->tokenizer.tokens.Advance(); // skip =
            now = PCLPP->tokenizer.tokens.Advance();
            if (isdigit(now[0]))
            {
                DivideImm(ih, b, stoi(now));
            }
            else
            {
                PCLPP_MemoryReference& mr2 = PCLPP->GetReference(now);
                IndexHolder& ih2 = Get(mr2.index, holders);
                DivideRR(ih, b, ih2.reg);
            }
        }

        now = PCLPP->tokenizer.tokens.Advance(); // skip semicolon
        if (now != ";")
        {
            PCLPP->tokenizer.tokens.iteration--;
        }
        now = PCLPP->tokenizer.tokens.Advance();
        if (now == "}") break;
    }

    // Restore Variables
    for (int i = 0; i < mrrs.size(); i++)
    {
        IndexHolder& ih = holders[i];
        ReloadVar(ih, mrrs[i], b, PCLPP);
    }
}