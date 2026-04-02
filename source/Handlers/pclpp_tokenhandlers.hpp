#pragma once

#include "pclpp_mainhandler.h"
#include "pclpp_returnhandler.h"
#include "pclpp_classhandler.h"
#include "pclpp_callhandler.h"
#include "pclpp_edithandler.h"
#include "pclpp_classfunctionhandler.h"
#include "pclpp_treathandler.h"
#include "pclpp_stringhandler.h"
#include "pclpp_subblockhandler.h"
#include <memory>

class PCLPP_TokenHandlers
{
public:
    std::vector<std::unique_ptr<PCLPP_TokenHandler>> handlers;

    void RegisterAll()
    {
        handlers.push_back(std::make_unique<PCLPP_MainHandler>());
        handlers.push_back(std::make_unique<PCLPP_ReturnHandler>());
        handlers.push_back(std::make_unique<PCLPP_ClassHandler>());
        handlers.push_back(std::make_unique<PCLPP_CallHandler>());
        handlers.push_back(std::make_unique<PCLPP_EditHandler>());
        handlers.push_back(std::make_unique<PCLPP_ClassFunctionHandler>());
        handlers.push_back(std::make_unique<PCLPP_TreatHandler>());
        handlers.push_back(std::make_unique<PCLPP_StringHandler>());
        handlers.push_back(std::make_unique<PCLPP_SubBlockHandler>());
    }

    void Call(PCLPP* PCLPP, std::string token)
    {
        for (auto& func : handlers)
        {
            func->OnToken(PCLPP, token);
        }
    }
};
