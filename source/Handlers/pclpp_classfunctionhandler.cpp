#include "pclpp_classfunctionhandler.h"
#include "../plcpp_compiler.hpp"

void LoadClassRecursive(PCLPP* PCLPP, PCLPP_Class& c, PCLPP_Block& b, PCLPP_MemoryReference* parent = nullptr)
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
            LoadClassRecursive(PCLPP,newc, b, child);
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
        }
    }
}

void PCLPP_ClassFunctionHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "function") return;
    if (PCLPP->inBlock) return;
    std::string ClassName;
    bool inl = false;
    loop:
    ClassName = PCLPP->tokenizer.tokens.Advance();
    if (ClassName == "inline")
    {
        inl = true;
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
    PCLPP_MemoryReference& mr = b.memoryReferences.emplace_back();
    mr.name = "this";
    mr.size = 4;
    mr.index = 0;
    mr.type = c.name;
    b.classvarcount++;
    LoadClassRecursive(PCLPP, c, b, &mr);
    b.type = PCLPP_Block_Type::Function;
    PCLPP->blocks.back().assembly.MOVRR(11, 14);
    PCLPP->blocks.back().assembly.PUSH(1 << 11);
    b.assembly.MOVRImm(0, -1);
}