// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/PlayerCharacter.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCharacter::StopInteractWithObject_Implementation() {}

AActor* APlayerCharacter::GetCurrentlyHighlightedActor_Implementation() { return nullptr; }

void APlayerCharacter::InteractWithActor_Implementation(AActor* actor_to_interact_with) {
	if (!actor_to_interact_with->Implements<UHighlightInterface>()) {
		return;
	}

	IHighlightInterface::Execute_Interact(actor_to_interact_with, this);
}

void APlayerCharacter::ReleaseInteractionInput_Implementation() {}
