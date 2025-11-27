// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemTheme.h"
#include "WrittingReviewGameInstance.h"
#include "ThemeElements/ThemeHolder.h"

void AItemTheme::BeginPlay()
{
    Super::BeginPlay();

    UWrittingReviewGameInstance* GI = Cast<UWrittingReviewGameInstance>(GetGameInstance());
    WeightMap= GI->TextScoringSystem->GetWeightsFromClass(this->GetClass());

}

void AItemTheme::UseObject_Implementation(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject) {
    if (!PlayerCharacter) {
        Super::UseObject_Implementation(UseDirection, UsePosition, ActorUsingObject);
        return;
    }

    auto actor_highlighted = Cast<AThemeHolder>(PlayerCharacter->GetCurrentlyHighlightedActor());
    if (!actor_highlighted) {
        Super::UseObject_Implementation(UseDirection, UsePosition, ActorUsingObject);
        return;
    }
    
    PlayerCharacter->StopInteractWithObject();
    Super::Interact_Implementation(actor_highlighted);
    actor_highlighted->Execute_Interact(actor_highlighted, this);
}

void AItemTheme::StopInteract_Implementation(AActor* ActorStopingInteract) {
    if (PlayerCharacter) {
        PlayerCharacter->PlayHoldingAnimationMontage(false, IsOneHanded);
        IsHeld = false;
    }
    auto owner = Cast<AThemeHolder>(GetAttachParentActor());
    if (owner) {
        return;
    }

    Super::StopInteract_Implementation(ActorStopingInteract);
}

