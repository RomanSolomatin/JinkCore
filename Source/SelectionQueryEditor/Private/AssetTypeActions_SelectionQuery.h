// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_SelectionQuery : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SelectionQuery", "Selection Query"); }
	virtual FColor GetTypeColor() const override { return FColor(129,50,255); }
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
	virtual uint32 GetCategories() override;
};