#pragma once

#include "pclpp_tokenhandler.hpp"

class PCLPP_ReturnHandler : public PCLPP_TokenHandler
{
    public:
    void OnToken(PCLPP* PCLPP, const std::string& token) override;
};