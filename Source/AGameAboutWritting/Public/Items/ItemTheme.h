// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "ItemTheme.generated.h"

/**
 * 
 */
UCLASS()
class AGAMEABOUTWRITTING_API AItemTheme : public AItem
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	FString const& GetThemeName() { return Theme; }

	UFUNCTION(BlueprintCallable)
		TMap<FString, float> const& GetThemeRelatedList() { return WeightMap; }

	virtual void UseObject_Implementation(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject) override;
	virtual void StopInteract_Implementation(AActor* ActorStopingInteract) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected :

	TMap<FString, float> WeightMap;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Theme;

	// Array of your param structs
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSet<FString> ThemeRelatedList;
};
