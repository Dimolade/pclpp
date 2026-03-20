#include "pclpp_classhandler.h"
#include "../plcpp_compiler.hpp"

void HandleClass(PCLPP* PCLPP, const std::string& token)
{
    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_Class& Class = PCLPP->classes.emplace_back();
    Class.name = name;

    std::string blockStart = PCLPP->tokenizer.tokens.Advance();
    if (blockStart == "byte")
    {
        Class.isByteClass = true;
        std::string next = PCLPP->tokenizer.tokens.Advance();
        uint8_t size = stoi(next);
        Class.byteSize = size;
        PCLPP->tokenizer.tokens.Advance(); // skip ;
        return;
    }
    if (blockStart != "{") return;
    std::string thisToken = PCLPP->tokenizer.tokens.Advance();
    uint16_t variableOffset = 0;
    while (thisToken != "}")
    {
        std::string type = thisToken;
        std::string name = PCLPP->tokenizer.tokens.Advance();
        std::string next = PCLPP->tokenizer.tokens.Advance();
        if (next == ";")
        {
            PCLPP_Variable& v = Class.variables.emplace_back();
            v.name = name;
            v.type = type;
            variableOffset += PCLPP->GetTypeSize(type);
            goto skip;
        }
        if (next == "=")
        {
            std::string value = PCLPP->tokenizer.tokens.Advance();
            PCLPP_Variable& v = Class.variables.emplace_back();
            v.name = name;
            v.type = type;
            if (isdigit(value[0]))
            {
                v.defaultValue = stoi(value);
                PCLPP->tokenizer.tokens.Advance(); // skip ';'
                variableOffset += PCLPP->GetTypeSize(type);
                goto skip;
            }
        }
        skip:
        thisToken = PCLPP->tokenizer.tokens.Advance();
    }
}

void InitClassRecursive(PCLPP_Class& cls, PCLPP_MemoryReference& parent, Assembly& assembly, PCLPP* PCLPP, PCLPP_Block& b)
{
    for (PCLPP_Variable& var : cls.variables)
    {
        PCLPP_Class& childClass = PCLPP->GetClass(var.type);

        PCLPP_MemoryReference& child = parent.children.emplace_back();

        child.name = var.name;
        child.size = PCLPP->GetTypeSize(var.type);
        child.index = PCLPP->localVarCount;

        if (childClass.isByteClass)
        {
            PCLPP->LoadClass(childClass, assembly);
            b.assembly.MOVRImm(0, PCLPP->localVarCount);
            b.assembly.CallFunction((uint32_t)pclpp_std::AllocateLocal);
            b.myLocals.push_back(PCLPP->localVarCount);
            PCLPP->localVarCount++;
        }
        else
        {
            PCLPP->localVarCount++;
            InitClassRecursive(childClass, child, assembly, PCLPP, b);
        }
    }
}

void HandleNew(PCLPP* PCLPP, const std::string& token)
{
    std::string Class = PCLPP->tokenizer.tokens.Advance();
    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_Block& b = PCLPP->blocks.back();
    PCLPP_MemoryReference& parent = b.memoryReferences.emplace_back();
    parent.name = name;
    parent.size = PCLPP->GetTypeSize(Class);
    PCLPP->tokenizer.tokens.Advance(); // skip ';'
    PCLPP_Class& c = PCLPP->GetClass(Class);
    PCLPP->LoadClass(c,b.assembly);
    b.assembly.MOVRR(10,0);
    b.assembly.PUSH(1 << 10);
    if (c.isByteClass)
    {
        b.assembly.POP(1 << 10);
        return;
    }
    InitClassRecursive(c, parent, b.assembly, PCLPP, b);
    b.assembly.POP(1 << 10);
    b.assembly.MOVRR(0,10);
    b.assembly.CallFunction((uint32_t)pclpp_std::AllocateLocal);
    parent.index = PCLPP->localVarCount;
    b.myLocals.push_back(PCLPP->localVarCount);
    PCLPP->localVarCount++;
}

void PCLPP_ClassHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token == "class")
    {
        HandleClass(PCLPP, token);
    }
    else if (token == "new")
    {
        HandleNew(PCLPP, token);
    }
    
}