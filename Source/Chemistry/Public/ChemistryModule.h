// Copyright 2015-2018 Piperift. All Rights Reserved.
#pragma once

#include "Private/ChemistryPrivatePCH.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

//#include "Developer/AssetTools/Public/IAssetTools.h"
//#include "Developer/AssetTools/Public/AssetToolsModule.h"
#endif //WITH_EDITOR

DECLARE_LOG_CATEGORY_EXTERN(LogChemistry, All, All);

class FChemistryModule : public IModuleInterface
{
public:

    // Get Jink Core module instance
    FORCEINLINE static FChemistryModule* GetInstance() { 
        return &FModuleManager::LoadModuleChecked<FChemistryModule>("Chemistry");
    }

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual bool SupportsDynamicReloading() override { return true; }

    DECLARE_DELEGATE_RetVal(void, FOnModifiedSettings)
    FOnModifiedSettings& OnModifiedSettings()
    {
        return ModifiedSettingsDelegate;
    }

private:
    /** Holds a delegate that is executed after the settings section has been modified. */
    FOnModifiedSettings ModifiedSettingsDelegate;

    void RegisterSettings();
    void UnregisterSettings();

    // Callbacks for when the settings were saved.
    bool HandleSettingsSaved();
};