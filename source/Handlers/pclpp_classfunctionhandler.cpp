#include "pclpp_classfunctionhandler.h"
#include "../plcpp_compiler.hpp"

void LoadClassRecursive(PCLPP* PCLPP, PCLPP_Class& c, PCLPP_Block& b, uint8_t isThis = 1, PCLPP_MemoryReference* parent = nullptr)
{
    for (PCLPP_Variable& v : c.variables)
    {
        PCLPP_Class& newc = PCLPP->GetClass(v.type);
        if (!newc.isByteClass)
        {
            PCLPP_MemoryReference* child = nullptr;
            if (parent == nullptr)
            {
                child = &b.memoryReferences.emplace_back();
            }
            else
                child = &parent->children.emplace_back();
            
            child->name = v.name;
            child->index = b.classvarcount;
            b.classvarcount++;
            child->size = 4;
            child->type = newc.name;
            child->partofthis = isThis;
            child->free = false;
            LoadClassRecursive(PCLPP,newc, b, isThis, child);
            continue;
        }
        else
        {
            PCLPP_MemoryReference* child = nullptr;
            if (parent == nullptr)
            {
                child = &b.memoryReferences.emplace_back();
            }
            else
                child = &parent->children.emplace_back();
            
            child->name = v.name;
            child->index = b.classvarcount;
            b.classvarcount++;
            child->size = newc.byteSize;
            child->type = newc.name;
            child->partofthis = isThis;
            child->free = false;
        }
    }
}

void SyncClass(PCLPP_MemoryReference& mr, uint8_t& reg, PCLPP_Class& c)
{

}

void PCLPP_ClassFunctionHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "function") return;
    if (PCLPP->inBlock) return;
    std::string ClassName;
    bool inl = false;
    bool noffset = false;
    bool noclean = false;
    bool nodefault = false;
    loop:
    ClassName = PCLPP->tokenizer.tokens.Advance();
    if (ClassName == "inline")
    {
        inl = true;
        goto loop;
    }
    else if (ClassName == "noffset")
    {
        noffset = true;
        goto loop;
    }
    else if (ClassName == "noclean")
    {
        noclean = true;
        goto loop;
    }
    else if (ClassName == "nodefault")
    {
        nodefault = true;
        goto loop;
    }
    PCLPP->tokenizer.tokens.Advance(); // skip "."
    PCLPP_Class& c = PCLPP->GetClass(ClassName);
    PCLPP_Class_Function& cf = c.functions.emplace_back();
    std::string FuncName = PCLPP->tokenizer.tokens.Advance();
    cf.name = FuncName;
    cf.blockIndex = PCLPP->blocks.size();
    cf.inl = inl;
    PCLPP->inBlock = true;
    PCLPP_Block& b = PCLPP->blocks.emplace_back();
    b.inl = inl;
    b.noffset = noffset;
    b.noclean = noclean;
    b.nodefault = nodefault;

    // load arguments
    PCLPP->tokenizer.tokens.Advance(); // skip (
    uint8_t argIndex = 5;
    std::string now = PCLPP->tokenizer.tokens.Advance();
    while (now != ")")
    {
        if (now == ",")
        {
            now = PCLPP->tokenizer.tokens.Advance();
            argIndex++;
            continue;
        }
        std::string className = now;
        std::string varName = PCLPP->tokenizer.tokens.Advance();
        PCLPP_Class& c = PCLPP->GetClass(className);
        PCLPP_MemoryReference& arg = b.memoryReferences.emplace_back();
        arg.name = varName;
        arg.partofthis = 0;
        arg.size = c.byteSize;
        arg.type = c.name;
        arg.index = PCLPP->localVarCount;
        now = PCLPP->tokenizer.tokens.Advance();
        if (now == "*")
        {
            arg.free = false;
            PCLPP->DuplicateVariable(arg, argIndex, c, b);
            now = PCLPP->tokenizer.tokens.Advance();
            continue;
        }
        PCLPP->NewLocalWithValue(b, arg.size, argIndex);
    }

    PCLPP_MemoryReference& mr = b.memoryReferences.emplace_back();
    mr.name = "this";
    mr.size = 4;
    mr.index = 0;
    mr.type = c.name;
    mr.partofthis = 1;
    mr.free = false;
    b.classvarcount++;
    LoadClassRecursive(PCLPP, c, b, 1, &mr);
    b.type = PCLPP_Block_Type::Function;
    if (!inl)
    {
        PCLPP->blocks.back().assembly.MOVRR(11, 14);
        PCLPP->blocks.back().assembly.PUSH(1 << 11);
    }
    if (!nodefault)
    {
        b.assembly.MOVRImm(0, -1);
    }
}