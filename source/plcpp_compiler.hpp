
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

class PCLPP
{
public:
    PCLPP_Tokenizer tokenizer;
    PCLPP_TokenHandlers handlers;
    std::vector<PCLPP_Block> blocks;
    std::vector<PCLPP_Class> classes;
    bool inBlock = false;
    uint16_t localVarCount = 0;

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

    void UnallocateBlock(PCLPP_Block& b)
    {
        for (uint16_t index : b.myLocals)
        {
            blocks.back().assembly.MOVRImm(0, index);
            blocks.back().assembly.CallFunction((uint32_t)pclpp_std::UnallocateLocal);
        }
    }

    void LoadClass(PCLPP_Class& c, Assembly& assembly)
    {
        if (c.isByteClass)
        {
            assembly.MOVRImm(0, c.byteSize);
            assembly.CallFunction((uint32_t)pclpp_std::Malloc);
        }
        uint32_t totalSize = 0;
        if (c.isByteClass)
        {
            totalSize = c.byteSize;
        }
        else
        {
            for (PCLPP_Variable& v : c.variables)
            {
                totalSize += GetTypeSize(v.type);
            }
        }
        assembly.MOVRImm(0,totalSize);
        assembly.CallFunction((uint32_t)pclpp_std::Malloc);
        assembly.MOVRR(0,10);
        assembly.PUSH(1 << 10);
        for (PCLPP_Variable& v : c.variables)
        {
            assembly.MOVRImm(1,v.defaultValue);
            uint8_t size = GetTypeSize(v.type);
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
            assembly.ADDRImm(0,size);
        }
        assembly.POP(1 << 10);
        assembly.MOVRR(0,10);
    }
};
