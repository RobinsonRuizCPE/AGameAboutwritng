// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Interface.h"
#include <EnhancedInput/Public/InputAction.h>
#include "HighlightInterface.generated.h"

/**
 * 
 */
UINTERFACE(Blueprintable)
class AGAMEABOUTWRITTING_API UInteractionInterface : public UInterface
{
	GENERATED_BODY()
	
};

class IInteractionInterface
{
    GENERATED_BODY()

public:

    // BlueprintCallable or BlueprintNativeEvent if you want BP override
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Highlight")
        void OnHitByPlayerLaser();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Highlight")
        void OnNoLongerHitByPlayerLaser();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        bool Interact(AActor* ActorThatInteract);

   UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        bool SecondaryUse(AActor* ActorThatInteract);

   UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        void HandleKeyInput(FKey key_uded, bool pressed);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        FString GetInteractionActionName();
    virtual FString GetInteractionActionName_Implementation() { return TEXT("Interact"); }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        FString GetUseActionName();
    virtual FString GetUseActionName_Implementation() { return TEXT("Use"); }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    FString GetSecondaryUseActionName();
    virtual FString GetSecondaryUseActionName_Implementation() { return TEXT(""); }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        FString GetCancelActionName();
    virtual FString GetCancelActionName_Implementation() { return TEXT("Cancel"); }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        bool ReleaseInteractionButton(AActor* ActorThatInteract);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        void StopInteract(AActor* ActorStopingInteract);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        void UseObject(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject);
};