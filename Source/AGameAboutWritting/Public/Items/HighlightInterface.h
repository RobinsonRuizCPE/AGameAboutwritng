// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

/**
 * 
 */
UINTERFACE(Blueprintable)
class AGAMEABOUTWRITTING_API UHighlightInterface : public UInterface
{
	GENERATED_BODY()
	
};

class IHighlightInterface
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
        void StopInteract(AActor* ActorStopingInteract);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
        void UseObject(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject);
};