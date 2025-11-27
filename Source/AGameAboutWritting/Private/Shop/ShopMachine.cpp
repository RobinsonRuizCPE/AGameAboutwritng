// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/ShopMachine.h"

// Sets default values
AShopMachine::AShopMachine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShopMachine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShopMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShopMachine::SendWidgetClickFromUV(
    UWidgetInteractionComponent* WidgetInteraction,
    UWidgetComponent* WidgetComp,
    const FVector2D& UV01,
    bool bFlipV)
{
    if (!WidgetInteraction || !WidgetComp)
    {
        return;
    }

    // 1. get widget size and pivot
    const FVector2D DrawSize = WidgetComp->GetDrawSize();
    const FVector2D Pivot = WidgetComp->GetPivot();

    float U = bFlipV ? (1.0f- UV01.X) : UV01.X;
    float V = bFlipV ? (1.0f - UV01.Y) : UV01.Y;

    // offset from pivot in pixels
    float OffsetY = (U - Pivot.X) * DrawSize.X;
    float OffsetZ = (V - Pivot.Y) * DrawSize.Y;

    // project those onto the component's local axes
    const FTransform& T = WidgetComp->GetComponentTransform();
    FVector WorldHitPos =
        T.GetLocation()
        + T.GetUnitAxis(EAxis::Y) * OffsetY
        + T.GetUnitAxis(EAxis::Z) * OffsetZ;

    const FVector WorldNormal = T.GetUnitAxis(EAxis::X); // widget forward (out of screen)

    // --- 4. Move the WidgetInteractionComponent to that point
    // (a few cm away, looking straight at the widget)
    FVector InteractionPos = WorldHitPos + WorldNormal * 200.0f; // push slightly in front
    WidgetInteraction->SetWorldLocation(InteractionPos);

    // Orient it so its forward vector points back at the widget
    FRotator LookAtRot = (-WorldNormal).Rotation();
    WidgetInteraction->SetWorldRotation(LookAtRot);
}

