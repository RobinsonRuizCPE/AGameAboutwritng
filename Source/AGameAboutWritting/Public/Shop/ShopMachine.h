// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/HitResult.h"
#include "../InteractableActor/ActorInteractable.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "ShopMachine.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API AShopMachine : public AActorInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShopMachine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// UV is 0..1 from your mesh hit
	UFUNCTION(BlueprintCallable, Category = "UI")
		static void SendWidgetClickFromUV(
			UWidgetInteractionComponent* WidgetInteraction,
			UWidgetComponent* WidgetComp,
			const FVector2D& UV01,
			bool bFlipV = true);
};
