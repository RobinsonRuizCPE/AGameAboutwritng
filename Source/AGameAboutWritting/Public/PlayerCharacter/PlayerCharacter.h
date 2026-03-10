// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Items/HighlightInterface.h"
#include "../Utility/EAnimBodyMask.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API APlayerCharacter : public ACharacter, public IInteractionInterface
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

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Animation")
		void PlayCustomAnimationMontage(UAnimMontage* anim_montage, bool activate, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AGameAboutWritting.EAnimBodyMask"))int32 Masks);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void ReleaseInteractionInput();
	virtual void ReleaseInteractionInput_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
	void SecondaryUseWithObject();
	virtual void SecondaryUseWithObject_Implementation();


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void StopInteractWithObject();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		AActor* GetCurrentlyHighlightedActor();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive")
		void InteractWithActor(AActor* actor_to_interact_with);
	virtual void InteractWithActor_Implementation(AActor* actor_to_interact_with);

	UFUNCTION(BlueprintCallable, Category = "Anim|Mask")
	void SetActiveAnimMasks(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AGameAboutWritting.EAnimBodyMask"))int32 NewMaskBits);


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	bool IsMovedByPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	bool IsPlayerHoldingItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	AActor* CurrentlyObjectInteracted = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Anim", meta = (Bitmask, BitmaskEnum = "/Script/AGameAboutWritting.EAnimBodyMask"))
	int32 ActiveAnimMaskBits = 0;
};
