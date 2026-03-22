
#include "pclpp_std.hpp"
#include "assemblinizer_jit.h"
#include "Handlers/pclpp_tokenhandlers.hpp"
#include <string>

class PCLPP_DataCont
{
public:
    std::vector<std::string> data;
    int iteration = 0;

    std::string Advance()
    {
        if (data.size() <= iteration) return "";
        std::string tor = data[iteration];
        iteration++;
        return tor;
    }

    void push_back(std::string pb)
    {
        data.push_back(pb);
    }
};

class PCLPP_Tokenizer
{
public:
    PCLPP_DataCont tokens;
    bool IsSeperate(char c)
    {
        switch (c)
        {
            case ']':
            case '[':
            case '}':
            case '{':
            case ')':
            case '(':
            case ',':
            case '.':
            case '/':
            case '*':
            case '+':
            case '-':
            case ':':
            case ';':
            case '#':
            case '<':
            case '>':
            case '\"':
            case '\'':
            case '\\':
            case '=':
                return true;
        }
        return false;
    }
    void tokenize(std::string in)
    {
        std::string buffer = "";
        for (int i = 0; i < in.length(); i++)
        {
            if (in[i] == ' ' || in[i] == '\n')
            {
                if (buffer != "")
                {
                    tokens.push_back(buffer);
                    buffer = "";
                }
                continue;
            }
            if (in[i] == '"')
            {
                i++;

                while (i < in.size())
                {
                    if (in[i] == '\\')
                    {
                        i++;
                        if (i >= in.size()) break;

                        switch (in[i])
                        {
                            case 'n': buffer += '\n'; break;
                            case 't': buffer += '\t'; break;
                            case '\\': buffer += '\\'; break;
                            case '"': buffer += '"'; break;
                            default: buffer += in[i]; break;
                        }
                    }
                    else if (in[i] == '"')
                    {
                        tokens.push_back(buffer);
                        buffer.clear();
                        break;
                    }
                    else
                    {
                        buffer += in[i];
                    }

                    i++;
                }

                continue;
            }
            if (IsSeperate(in[i]))
            {
                if (buffer != "")
                {
                    tokens.push_back(buffer);
                    buffer = "";
                }
                tokens.push_back(std::string("")+in[i]);
                continue;
            }
            buffer += in[i];
        }
        if (buffer != "")
        {
            tokens.push_back(buffer);
            buffer = "";
        }
    }
};

enum PCLPP_Block_Type
{
    Function,
    Main
};

class PCLPP_MemoryReference
{
public:
    uint16_t index;
    uint8_t size;
    std::string name;
    std::vector<PCLPP_MemoryReference> children;
};

class PCLPP_Block
{
public:
    PCLPP_Block_Type type = PCLPP_Block_Type::Function;
    std::vector<PCLPP_MemoryReference> memoryReferences;
    Assembly assembly;
    std::vector<uint16_t> myLocals;
};

class PCLPP_Variable
{
public:
    uint32_t defaultValue = 0;
    std::string type = "int4";
    std::string name = "__error";
    uint32_t offset = 0;
};

class PCLPP_Class
{
public:
    std::vector<PCLPP_Variable> variables;
    std::vector<PCLPP_Block> blocks;
    bool isByteClass = false;
    uint8_t byteSize = 0;
    std::string name;
};

class PCLPP_Library_Link
{
public:
    std::string name;
    std::string name_space;
    uint32_t address;
    int size;
};

class PCLPP_Library
{
public:
    std::vector<PCLPP_Library_Link> links;
    void Link(std::string name, uint32_t address, uint8_t size = 4)
    {
        Link(name, "", address, size);
    }
    void Link(std::string name, std::string namespace, uint32_t address, uint8_t size = 4)
    {
        PCLPP_Library_Link& l = links.emplace_back();
        l.name = name;
        l.name_space = namespace;
        l.address = address;
        l.size = size;
    }
};

class PCLPP
{
public:
    PCLPP_Tokenizer tokenizer;
    PCLPP_TokenHandlers handlers;
    std::vector<PCLPP_Block> blocks;
    std::vector<PCLPP_Class> classes;
    std::vector<PCLPP_Library&> libraries;
    bool inBlock = false;
    uint16_t localVarCount = 0;

    void AddLibrary(PCLPP_Library& l)
    {
        libraries.push_back(l);
    }

    uint32_t GetTypeSize(std::string type)
    {
        for (PCLPP_Class& c : classes)
        {
            if (c.name == type)
            {
                uint32_t totalSize = 0;
                if (c.isByteClass)
                {
                    return c.byteSize;
                }
                else
                {
                    for (PCLPP_Variable& v : c.variables)
                    {
                        totalSize += GetTypeSize(v.type);
                    }
                }
                return totalSize;
            }
        }
        return 0;
    }

    Assembly& GetMainAssembly()
    {
        for (PCLPP_Block& b : blocks)
        {
            if (b.type == PCLPP_Block_Type::Main)
            {
                return b.assembly;
            }
        }
        return;
    }

    PCLPP_Block& GetMain()
    {
        for (PCLPP_Block& b : blocks)
        {
            if (b.type == PCLPP_Block_Type::Main)
            {
                return b;
            }
        }
        return;
    }

    void compile(std::string in)
    {
        tokenizer.tokenize(in);
        handlers.RegisterAll();
        while (tokenizer.tokens.iteration < tokenizer.tokens.data.size())
        {
            std::string t = tokenizer.tokens.Advance();
            if (!t.empty()) handlers.Call(this, t);
        }
    }

    PCLPP_Library_Link& GetAddress(std::string name, std::string name_space = "")
    {
        for (PCLPP_Library& l : libraries)
        {
            for (PCLPP_Library_Link& ll : l.links)
            {
                if (ll.name_space == name_space && ll.name == name)
                {
                    return ll;
                }
            }
        }
        return;
    }

    PCLPP_MemoryReference& GetReferenceRecursive(PCLPP_MemoryReference& mrr, std::string token)
    {
        PCLPP_MemoryReference* cand = nullptr;

        for (PCLPP_MemoryReference& mr : mrr.children)
        {
            if (mr.name == token)
            {
                cand = &mr;
                break;
            }
        }

        std::string next = tokenizer.tokens.Advance();

        if (next == ".")
        {
            return GetReferenceRecursive(*cand, tokenizer.tokens.Advance(), pclpp);
        }

        tokenizer.tokens.iteration--;
        return *cand;
    }

    PCLPP_MemoryReference& GetReference(std::string token)
    {
        PCLPP_Block& thisBlock = blocks.back();
        PCLPP_MemoryReference* cand;
        for (PCLPP_MemoryReference& mr : thisBlock.memoryReferences)
        {
            if (mr.name == token)
            {
                cand = &mr;
                break;
            }
        }
        std::string next = tokenizer.tokens.Advance();
        if (next == ".")
        {
            return GetReferenceRecursive(*cand, tokenizer.tokens.Advance());
        }
        tokenizer.tokens.iteration--;
        return *cand;
    }

    void LoadString(std::string string, Assembly& assembly) // loads string into r0
    {
        assembly.MOVRImm(0, string.length());
        assembly.MOVRImm(1, 1);
        assembly.CallFunction((uint32_t)pclpp_std::Calloc);
        assembly.MOVRR(10,0);
        assembly.PUSH(1 << 10);
        if (string.length() >= 1)
        {
            assembly.MOVRImm(1, string[0]);
            assembly.CallFunction((uint32_t)pclpp_std::Write8);
        }
        for (int i = 1; i < string.length(); i++)
        {
            assembly.MOVRImm(1, string[i]);
            assembly.ADDRImm(0, 1);
            assembly.CallFunction((uint32_t)pclpp_std::Write8);
        }
        assembly.POP(1 << 10);
        assembly.MOVRR(0,10);
    }

    PCLPP_Class& GetClass(const std::string& name)
    {
        for (PCLPP_Class& c : classes)
        {
            if (c.name == name) return c;
        }
        return;
    }

    void UnallocateMR(PCLPP_Block& b, std::vector<PCLPP_MemoryReference>& mr)
    {
        for (PCLPP_MemoryReference& mrr : mr)
        {
            blocks.back().assembly.MOVRImm(0, mrr.index);
            blocks.back().assembly.CallFunction((uint32_t)pclpp_std::GetLocal);
            blocks.back().assembly.CallFunction((uint32_t)pclpp_std::Free);
            UnallocateMR(b, mrr.children);
        }
    }

    void UnallocateBlock(PCLPP_Block& b)
    {
        UnallocateMR(b, b.memoryReferences);
        for (uint16_t index : b.myLocals)
        {
            blocks.back().assembly.MOVRImm(0, index);
            blocks.back().assembly.CallFunction((uint32_t)pclpp_std::UnallocateLocal);
        }
    }

    void NewLocal(PCLPP_Block& b)
    {
        b.assembly.PUSH(1 << 0);
        b.assembly.CallFunction((uint32_t)pclpp_std::AllocateLocal);
        b.assembly.POP(1 << 0);
        b.myLocals.push_back(localVarCount);
        localVarCount++;
    }

    void NewLocalWithValue(PCLPP_Block& b, uint8_t size)
    {
        b.assembly.PUSH(1 << 1);
        b.assembly.PUSH(1 << 0);
        b.assembly.MOVRR(1, 0);
        b.assembly.MOVRImm(0, size);
        b.assembly.CallFunction((uint32_t)pclpp_std::Malloc);
        // r0: address, r1: value
        switch (size)
        {
            case 1:
            b.assembly.CallFunction((uint32_t)pclpp_std::Write8);
            break;
            case 2:
            b.assembly.CallFunction((uint32_t)pclpp_std::Write16);
            break;
            case 4:
            b.assembly.CallFunction((uint32_t)pclpp_std::Write32);
            break;
        }
        b.assembly.POP(1 << 0);
        b.assembly.POP(1 << 1);
        // original values restored
    }

    void LoadByteClass(PCLPP_Class& c, Assembly& assembly, PCLPP_Block& b, uint32_t value = 0)
    {
        if (!c.isByteClass) return;
        assembly.MOVRImm(0, c.byteSize);
        assembly.CallFunction((uint32_t)pclpp_std::Malloc);
        assembly.MOVRR(10, 0);
        assembly.PUSH(1 << 10);
        NewLocal(b);
        assembly.POP(1 << 10);
        assembly.MOVRR(0,10);
        assembly.PUSH(1 << 10);
        assembly.MOVRImm(1, value);
        uint8_t size = c.byteSize;
        switch (size)
        {
            case 1:
            assembly.CallFunction((uint32_t)pclpp_std::Write8);
            break;
            case 2:
            assembly.CallFunction((uint32_t)pclpp_std::Write16);
            break;
            case 4:
            assembly.CallFunction((uint32_t)pclpp_std::Write32);
            break;
        }
        assembly.POP(1 << 10);
        assembly.MOVRR(0,10);
    }

    void LoadClassAsAddress(PCLPP_Class& c, Assembly& assembly, PCLPP_Block& b)
    {
        if (c.isByteClass) return;
        assembly.MOVRImm(0, 4); // address size is 32 bits (4 bytes)
        assembly.CallFunction((uint32_t)pclpp_std::Malloc);
        assembly.PUSH(1 << 0);
        NewLocal(b);
        assembly.POP(1 << 0);
        assembly.MOVRR(1,0);
        assembly.MOVRR(10,0);
        assembly.PUSH(1 << 10);
        assembly.CallFunction((uint32_t)pclpp_std::Write32);
        assembly.POP(1 << 10);
        assembly.MOVRR(0, 10);
    }
};
