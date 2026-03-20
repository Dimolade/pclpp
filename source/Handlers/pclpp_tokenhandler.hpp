#pragma once

#include <string>
#include <vector>

class PCLPP;

using PCLPP_TokenFunc = void(*)(PCLPP* PCLPP, std::string token);

class PCLPP_TokenHandler
{
public:
    virtual ~PCLPP_TokenHandler() = default;
    virtual void OnToken(PCLPP* PCLPP, const std::string& token) {}
};