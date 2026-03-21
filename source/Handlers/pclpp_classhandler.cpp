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

void LoadClassRecursive(PCLPP* PCLPP, PCLPP_Class& c, PCLPP_Block& b, PCLPP_MemoryReference& parent)
{
    for (PCLPP_Variable& v : c.variables)
    {
        PCLPP_Class& newc = PCLPP->GetClass(v.type);
        if (!newc.isByteClass)
        {
            PCLPP_MemoryReference& child = parent.children.emplace_back();
            child.name = v.name;
            child.index = PCLPP->localVarCount;
            child.size = 4;
            PCLPP->LoadClassAsAddress(newc, b.assembly, b);
            LoadClassRecursive(PCLPP,newc, b, child);
            continue;
        }
        else
        {
            PCLPP_MemoryReference& child = parent.children.emplace_back();
            child.name = v.name;
            child.index = PCLPP->localVarCount;
            child.size = newc.byteSize;
            PCLPP->LoadByteClass(newc, b.assembly, b, v.defaultValue);
        }
    }
}

void HandleNew(PCLPP* PCLPP, const std::string& token)
{
    std::string Class = PCLPP->tokenizer.tokens.Advance();
    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_Block& b = PCLPP->blocks.back();
    PCLPP_Class& c = PCLPP->GetClass(Class);
    PCLPP_MemoryReference& parent = b.memoryReferences.emplace_back();
    parent.name = name;
    if (c.isByteClass)
    {
        parent.size = c.byteSize;
        parent.index = PCLPP->localVarCount;
        PCLPP->LoadByteClass(c, b.assembly, b, 0);
        return;
    }
    else
    {
        parent.size = 4;
        parent.index = PCLPP->localVarCount;
        PCLPP->LoadClassAsAddress(c, b.assembly, b);
        LoadClassRecursive(PCLPP, c, b, parent);
    }
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