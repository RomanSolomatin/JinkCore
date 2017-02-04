// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ProceduralPrivatePCH.h"

// Settings
#include "ProceduralSettings.h"

DEFINE_LOG_CATEGORY(LogProcedural)

#define LOCTEXT_NAMESPACE "ProceduralModule"

void FProceduralModule::StartupModule()
{
    UE_LOG(LogProcedural, Warning, TEXT("Procedural: Log Started"));

    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    
    RegisterSettings();
}

void FProceduralModule::ShutdownModule()
{
    UE_LOG(LogProcedural, Warning, TEXT("Procedural: Log Ended"));
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    if (UObjectInitialized())
    {
        UnregisterSettings();
    }
}

void FProceduralModule::RegisterSettings()
{
#if WITH_EDITOR
    // Registering some settings is just a matter of exposing the default UObject of
    // your desired class, feel free to add here all those settings you want to expose
    // to your LDs or artists.

    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        // Get Project Settings category
        ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

        // Register Procedural settings
        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Game", "Procedural",
            LOCTEXT("RuntimeProceduralSettingsName", "Procedural"),
            LOCTEXT("RuntimeProceduralDescription", "Procedural configuration of Jink core."),
            GetMutableDefault<UProceduralSettings>());

        // Register the save handler to your settings, you might want to use it to
        // validate those or just act to settings changes.
        if (SettingsSection.IsValid())
        {
            SettingsSection->OnModified().BindRaw(this, &FProceduralModule::HandleSettingsSaved);
        }
    }
#endif
}

void FProceduralModule::UnregisterSettings()
{
#if WITH_EDITOR
    // Ensure to unregister all of your registered settings here, hot-reload would
    // otherwise yield unexpected results.

    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Game", "Procedural");
    }
#endif
}

bool FProceduralModule::HandleSettingsSaved()
{
    UProceduralSettings* Settings = GetMutableDefault<UProceduralSettings>();
    bool ResaveSettings = false;

    if (ModifiedSettingsDelegate.IsBound()) {
        ModifiedSettingsDelegate.Execute();
    }

    // You can put any validation code in here and resave the settings in case an invalid
    // value has been entered

    if (ResaveSettings)
    {
        Settings->SaveConfig();
    }
    
    return true;
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FProceduralModule, Procedural)