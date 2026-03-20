
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
    }
};

class PCLPP
{
public:
    Assembly assembly;
    PCLPP_Tokenizer tokenizer;
    PCLPP_TokenHandlers handlers;

    void compile(std::string in)
    {
        puts(in.c_str());
        tokenizer.tokenize(in);
        for (const std::string& token : tokenizer.tokens.data)
        {
            puts(token.c_str());
        }
        handlers.RegisterAll();
        while (tokenizer.tokens.iteration < tokenizer.tokens.data.size())
        {
            std::string t = tokenizer.tokens.Advance();
            if (!t.empty()) handlers.Call(this, t);
        }
    }

    void LoadString(std::string string) // loads string into r0
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
};
