#include "pclpp_callhandler.h"
#include "../plcpp_compiler.hpp"

void PCLPP_CallHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "call") return;
    if (PCLPP->inBlock == false) return;

    uint32_t addressToCall = 0;
    std::string namespaceName = "";
    std::vector<std::string> functionNames;
    std::string funcName = "";
    std::string outvarName = "";

    std::vector<std::string> collection;
    collection.push_back(PCLPP->tokenizer.tokens.Advance());
    while (collection.back() != "(")
    {
        collection.push_back(PCLPP->tokenizer.tokens.Advance());
    } // example: varName functionName ( <- Minimal

    bool isFunc = false;

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
        functionNames.push_back(namespaceName);
    }

    collection.clear();

    uint8_t size = 4;
    if (isFunc == false)
    {
        isFunc = PCLPP->AddressIsClass(funcName, namespaceName);
    }

    uint8_t argIndex = isFunc ? 5 : 0;
    std::string now = PCLPP->tokenizer.tokens.Advance();
    PCLPP->blocks.back().assembly.PUSH(1 << 0);
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
            PCLPP->blocks.back().assembly.PUSHUNSAFE();
            PCLPP->blocks.back().assembly.MOVRImm(0, mr.index);
            PCLPP->blocks.back().assembly.MOVRImm(1, mr.partofthis);
            PCLPP->blocks.back().assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
            std::string next = PCLPP->tokenizer.tokens.Advance(); // either ; or *
            if (next == ",")
            {
                PCLPP->ReadASM(mr.size, PCLPP->blocks.back());
                PCLPP->tokenizer.tokens.iteration--;
            }
            PCLPP->blocks.back().assembly.MOVRR(argIndex, 0);
            PCLPP->blocks.back().assembly.POPUNSAFE();
            now = PCLPP->tokenizer.tokens.Advance();
            continue;
        }
    }

    if (!isFunc)
    {
        PCLPP_Library_Link& add = PCLPP->GetAddress(funcName, namespaceName);
        PCLPP->blocks.back().assembly.CallFunction(add.address);
        size = add.size;
    }
    else
    {
        PCLPP->CallClassFunction(namespaceName, funcName, PCLPP->blocks.back());
    }

    if (outvarName != "")
    {
        PCLPP->blocks.back().assembly.PUSH(1 << 0); // r0: output from function
        PCLPP_MemoryReference& MR = PCLPP->blocks.back().memoryReferences.emplace_back();
        MR.name = outvarName;
        MR.index = PCLPP->localVarCount;
        MR.size = size;
        PCLPP->NewLocalWithValue(PCLPP->blocks.back(), size); // r0: ??
        PCLPP->blocks.back().assembly.POP(1 << 0); // r0: output from function
    }
    PCLPP->blocks.back().assembly.POP(1 << 0); // r0: previous value
    PCLPP->tokenizer.tokens.Advance(); // skip ;
}