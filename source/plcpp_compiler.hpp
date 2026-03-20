
#include "pclpp_std.hpp"
#include "assemblinizer.hpp"
#include <string>

class PLCPP_DataCont
{
private:
    int iteration = 0;
public:
    std::vector<std::string> data;

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

class PLCPP_Tokenizer
{
public:
    PLCPP_DataCont tokens;
    void tokenize(std::string in)
    {

    }
};

class PLCPP
{
public:
    Assembly assembly;
    PCLPP_Tokenizer tokenizer;

    void compile(std::string in)
    {
        tokenizer.tokenize(in);
    }
};
