// Copyright 2015-2017 Piperift. All Rights Reserved.

#pragma once

#include "Item.h"
#include "SQExecution.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROCEDURAL_API FSQExecution
{
public:
    GENERATED_USTRUCT_BODY()

    FSQExecution() : Weight(1), IsValid(false) {}

    FSQExecution(TSubclassOf<UItem> Item, float InWeight) :
        ItemValue(Item),
        Weight(InWeight),
        IsValid(true) 
    {
        AddItemToRecord(Item);
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    bool IsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    float Weight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    TSubclassOf<UItem> ItemValue;

    UPROPERTY(Transient)
    TSet<TSubclassOf<UItem>> ItemRecord;

    void AddItemToRecord(TSubclassOf<UItem> Item) {
        ItemRecord.Add(Item);
    }

    void AddItemsToRecord(TSet<TSubclassOf<UItem>> ItemSet) {
        ItemRecord.Append(ItemSet);
    }
};

USTRUCT(BlueprintType)
struct PROCEDURAL_API FPTClassExecution
{
public:
    GENERATED_USTRUCT_BODY()

    FPTClassExecution() : Weight(1), IsValid(false) {}

    FPTClassExecution(UClass* Item, float InWeight) :
        ItemValue(Item),
        Weight(InWeight),
        IsValid(true)
    {
        AddItemToRecord(Item);
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    bool IsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    float Weight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    UClass* ItemValue;

    UPROPERTY(Transient)
    TSet<UClass*> ItemRecord;

    void AddItemToRecord(UClass* Item) {
        ItemRecord.Add(Item);
    }

    void AddItemsToRecord(TSet<UClass*> ItemSet) {
        ItemRecord.Append(ItemSet);
    }
};

USTRUCT(BlueprintType)
struct PROCEDURAL_API FPTDataAssetExecution
{
public:
    GENERATED_USTRUCT_BODY()

    FPTDataAssetExecution() : Weight(1), IsValid(false) {}

    FPTDataAssetExecution(TAssetPtr<UDataAsset> Item, float InWeight) :
        ItemValue(Item),
        Weight(InWeight),
        IsValid(true)
    {
        AddItemToRecord(Item);
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    bool IsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    float Weight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    TAssetPtr<UDataAsset> ItemValue;

    UPROPERTY(Transient)
    TSet<TAssetPtr<UDataAsset>> ItemRecord;

    void AddItemToRecord(TAssetPtr<UDataAsset> Item) {
        ItemRecord.Add(Item);
    }

    void AddItemsToRecord(const TSet<TAssetPtr<UDataAsset>> ItemSet) {
        ItemRecord.Append(ItemSet);
    }
};
