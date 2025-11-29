// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Items/HighlightInterface.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API APlayerCharacter : public ACharacter, public IHighlightInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
		void PlayHoldingAnimationMontage(bool activate, bool one_handed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void ReleaseInteractionInput();
	virtual void ReleaseInteractionInput_Implementation();


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void StopInteractWithObject();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		AActor* GetCurrentlyHighlightedActor();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void InteractWithActor(AActor* actor_to_interact_with);
	virtual void InteractWithActor_Implementation(AActor* actor_to_interact_with);
};
