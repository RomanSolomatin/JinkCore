// Copyright 2015-2017 Piperift. All Rights Reserved.

#include "JinkEditorPrivatePCH.h"

#include "AnchorTypeCustomization.h"

#include "Asset/AssetTypeAction_LevelInstance.h"
#include "Asset/AssetTypeAction_Item.h"
#include "Asset/AssetTypeAction_SQ.h"
#include "Asset/AssetTypeAction_Buff.h"

#include "ContentBrowserExtensions/ContentBrowserExtensions.h"

DEFINE_LOG_CATEGORY(JinkEditor)
 
#define LOCTEXT_NAMESPACE "JinkEditor"
 
void FJinkEditorModule::StartupModule()
{
    UE_LOG(JinkEditor, Warning, TEXT("JinkEditor: Log Started"));

    RegisterPropertyTypeCustomizations();

    // Register asset types
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeAction_LevelInstance));
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeAction_Item));
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeAction_SQ));
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeAction_Buff));

    // Integrate JinkCore actions into existing editor context menus
    if (!IsRunningCommandlet())
    {
        FJCContentBrowserExtensions::InstallHooks();
    }
}
 
void FJinkEditorModule::ShutdownModule()
{
    UE_LOG(JinkEditor, Warning, TEXT("JinkEditor: Log Ended"));

    // Unregister all the asset types
    if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
    {
        IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
        {
            AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
        }
    }
    CreatedAssetTypeActions.Empty();


    if (UObjectInitialized())
    {
        FJCContentBrowserExtensions::RemoveHooks();
    }
}


void FJinkEditorModule::RegisterPropertyTypeCustomizations()
{
    RegisterCustomPropertyTypeLayout("LIAnchorType", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAnchorTypeCustomization::MakeInstance));
}


void FJinkEditorModule::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
    check(PropertyTypeName != NAME_None);

    static FName PropertyEditor("PropertyEditor");
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
    PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE(FJinkEditorModule, JinkEditor);