#include "pclpp_callhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_CallHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "call") return;
    if (PCLPP->inBlock == false) return;

    uint32_t addressToCall = 0;
    std::string namespaceName = "";
    std::string funcName = "";
    std::string outvarName = "";

    std::vector<std::string> collection;
    collection.push_back(PCLPP->tokenizer.tokens.Advance());
    while (collection.back() != "(")
    {
        collection.push_back(PCLPP->tokenizer.tokens.Advance());
    } // example: varName functionName ( <- Minimal

    if (collection.size() == 3)
    {
        outvarName = collection[0];
        funcName = collection[1];
    }
    else if (collection.size() == 2)
    {
        funcName = collection[0];
    }
    else if (collection.size() == 4)
    {
        namespaceName = collection[0];
        funcName = collection[2];
    }
    else if (collection.size() == 5)
    {
        outvarName = collection[0];
        namespaceName = collection[1]; // dot follows
        funcName = collection[3];
    }

    collection.clear();

    int argIndex = 0;
    std::string now = PCLPP->tokenizer.tokens.Advance();
    while (now != ")")
    {
        if (now == ",")
        {
            now = PCLPP->tokenizer.tokens.Advance();
            argIndex++;
            continue;
        }
        if (isdigit(now[0]))
        {
            PCLPP->blocks.back().assembly.MOVRImm(argIndex, stoi(now));
            now = PCLPP->tokenizer.tokens.Advance();
            continue;
        }
        else
        {
            PCLPP_MemoryReference& mr = PCLPP->GetReference(now);
            PCLPP->blocks.back().assembly.PUSH(1 << 0);
            PCLPP->blocks.back().assembly.MOVRImm(0, mr.index);
            PCLPP->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
            std::string next = PCLPP->tokenizer.tokens.Advance(); // either ; or *
            if (next == ",")
            {
                switch (mr.size)
                {
                    case 1:
                    PCLPP->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read8);
                    break;
                    case 2:
                    PCLPP->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read16);
                    break;
                    case 4:
                    PCLPP->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Read32);
                    break;
                }
                PCLPP->tokenizer.tokens.iteration--;
            }
            PCLPP->blocks.back().assembly.MOVRR(argIndex, 0);
            PCLPP->blocks.back().assembly.POP(1 << 0);
            now = PCLPP->tokenizer.tokens.Advance();
            continue;
        }
    }

    if (!PCLPP->AddressIsClass(funcName, namespaceName))
    {
        PCLPP_Library_Link& add = PCLPP->GetAddress(funcName, namespaceName);
        PCLPP->blocks.back().assembly.CallFunction(add.address);
    }
    else
    {
        PCLPP->CallClassFunction(namespaceName, funcName, PCLPP->blocks.back());
    }

    if (outvarName != "")
    {
        PCLPP->blocks.back().assembly.PUSH(1 << 0);
        PCLPP_MemoryReference& MR = PCLPP->blocks.back().memoryReferences.emplace_back();
        MR.name = outvarName;
        MR.index = PCLPP->localVarCount;
        MR.size = add.size;
        PCLPP->NewLocalWithValue(PCLPP->blocks.back(), add.size); // r0: value
        PCLPP->blocks.back().assembly.POP(1 << 0);
    }
    PCLPP->tokenizer.tokens.Advance(); // skip ;
}