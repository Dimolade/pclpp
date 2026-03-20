#pragma once

#include "pclpp_mainhandler.h"
#include "pclpp_returnhandler.h"
#include <memory>

class PCLPP_TokenHandlers
{
public:
    std::vector<std::unique_ptr<PCLPP_TokenHandler>> handlers;

    void RegisterAll()
    {
        handlers.push_back(std::make_unique<PCLPP_MainHandler>());
        handlers.push_back(std::make_unique<PCLPP_ReturnHandler>());
    }

    void Call(PCLPP* PCLPP, std::string token)
    {
        for (auto& func : handlers)
        {
            func->OnToken(PCLPP, token);
        }
    }
};
