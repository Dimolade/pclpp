#include "pclpp_mainhandler.h"
#include "../assemblinizer_jit.h"
#include "../plcpp_compiler.hpp"

#define mainhandler_debug

void TreatAs(PCLPP_MemoryReference& mr, PCLPP* PCLPP)
{
    std::string ClassName = PCLPP->tokenizer.tokens.Advance();
    PCLPP_Class& c = PCLPP->GetClass(ClassName);

    if (c.isByteClass)
    {
        mr.type = c.name;
        mr.size = c.byteSize
    }
}

void TreatWith(PCLPP_MemoryReference& mr, PCLPP* PCLPP)
{
    std::string nextOp = PCLPP->tokenizer.tokens.Advance();

    if (nextOp == "size")
    {
        std::string after = PCLPP->tokenizer.tokens.Advance();
        mr.size = std::stoi(after);
    }
    else if (nextOp == "index")
    {
        std::string after = PCLPP->tokenizer.tokens.Advance();
        mr.index = std::stoi(after);
    }
    else if (nextOp == "free")
    {
        std::string after = PCLPP->tokenizer.tokens.Advance();
        if (after == "true")
        {
            mr.free = true;
        }
        else if (after == "false")
        {
            mr.free = false;
        }
    }
    else if (nextOp == "offset")
    {
        std::string after = PCLPP->tokenizer.tokens.Advance();
        if (after == "true")
        {
            mr.partofthis = 1;
        }
        else if (after == "false")
        {
            mr.partofthis = 0;
        }
    }
}

void PCLPP_MainHandler::OnToken(PCLPP* PCLPP, const std::string& token)
{
    if (token != "treat") return;
    if (PCLPP->inBlock == false) return;

    std::string name = PCLPP->tokenizer.tokens.Advance();
    PCLPP_MemoryReference& mr = PCLPP->GetReference(name);

    std::string ope = PCLPP->tokenizer.tokens.Advance();

    if (ope == "as")
        TreatAs(mr, PCLPP);
    else if (ope == "with")
        TreatWith(mr, PCLPP);

    PCLPP->tokenizer.tokens.Advance(); // skip ;
}