// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RuntimeMaterialRegistry.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AGAMEABOUTWRITTING_API URuntimeMaterialRegistry : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static URuntimeMaterialRegistry * Get()
    {
        static URuntimeMaterialRegistry* Instance = nullptr;
        if (!Instance)
        {
            Instance = NewObject<URuntimeMaterialRegistry>();
            Instance->AddToRoot(); // prevent GC
        }
        return Instance;
    }

    UFUNCTION(BlueprintCallable)
    void RegisterMaterial(const FString & Key, UMaterialInterface * Material)
    {
        Registry.Add(Key, Material);
    }

    UFUNCTION(BlueprintCallable)
    void ClearRegistry()
    {
        Registry.Empty();
    }

    UMaterialInterface* FindMaterial(const FString& Key)
    {
        if (UMaterialInterface** Found = Registry.Find(Key))
        {
            return *Found;
        }
        return nullptr;
    }

private:
    TMap<FString, UMaterialInterface*> Registry;
};
