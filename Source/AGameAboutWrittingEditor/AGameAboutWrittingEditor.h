#pragma once

#include "Modules/ModuleManager.h"

class FAGameAboutWrittingEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
