// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Item.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the default subobject!
	ItemStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemStaticMesh"));
	RootComponent = ItemStaticMesh;
	ItemStaticMesh->BodyInstance.bUseCCD = true;

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(RootComponent);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);
}


// ===== CLASS GETTERS =====

FText AItem::GetItemDisplayNameFromClass(TSubclassOf<AItem> ItemClass)
{
	if (!ItemClass) return FText::GetEmpty();
	const AItem* CDO = ItemClass->GetDefaultObject<AItem>();
	return CDO ? CDO->GetDisplayName() : FText::GetEmpty();
}

int32 AItem::GetItemPriceFromClass(TSubclassOf<AItem> ItemClass)
{
	if (!ItemClass) return 0;
	const AItem* CDO = ItemClass->GetDefaultObject<AItem>();
	return CDO ? CDO->GetPrice() : 0;
}

UStaticMesh* AItem::GetItemMeshFromClass(TSubclassOf<AItem> ItemClass)
{
	if (!ItemClass) return nullptr;
	const AItem* CDO = ItemClass->GetDefaultObject<AItem>();
	return CDO ? CDO->GetMeshReference() : nullptr;
}

void AItem::OnHitByPlayerLaser_Implementation()
{
	UMaterialInterface* HighlightMat = LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/FXMaterials/OverlayFx/M_DestroyableObjectOverlayMat.M_DestroyableObjectOverlayMat") );
	ItemStaticMesh->SetOverlayMaterial(HighlightMat);
}

void AItem::OnNoLongerHitByPlayerLaser_Implementation()
{
	ItemStaticMesh->SetOverlayMaterial(nullptr);
}

bool AItem::Interact_Implementation(AActor* ActorThatInteract){
    if (!ActorThatInteract)
        return false;

    if (Sphere)
    {
        Sphere->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1,
            ECR_Ignore
        );
    }

    // 3. Stop physics on the mesh
    if (ItemStaticMesh)
    {
        ItemStaticMesh->SetSimulatePhysics(false);
    }

    // 2. Try casting the actor to your character class
    auto const interacter = Cast<APlayerCharacter>(ActorThatInteract);
    if (!interacter)
    {
        return false;
    }

    PlayerCharacter = interacter;
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
    AttachToComponent(
		PlayerCharacter->GetMesh(),      // parent component
        AttachRules,
        TEXT("hand_rSocket")
    );

    // 6. Set game state booleans
	PlayerCharacter->PlayHoldingAnimationMontage(true, IsOneHanded);
    IsHeld = true;    // if your item has an IsHeld variable
    return true;
}

void AItem::StopInteract_Implementation(AActor* ActorStopingInteract) {
    if (!ActorStopingInteract)
        return;

    if (Sphere)
    {
        Sphere->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1,
            ECR_Block
        );
    }

    // 3. Stop physics on the mesh
    if (ItemStaticMesh)
    {
        ItemStaticMesh->SetSimulatePhysics(true);
    }

    // 2. Try casting the actor to your character class
    PlayerCharacter = Cast<APlayerCharacter>(ActorStopingInteract);
    if (!PlayerCharacter)
    {
        return;
    }

    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    DetachFromActor(DetachRules);
    PlayerCharacter->PlayHoldingAnimationMontage(false, IsOneHanded);
    IsHeld = false;
}

void AItem::UseObject_Implementation(FVector UseDirection, FVector UsePosition, AActor* ActorUsingObject) {
    if (!PlayerCharacter)
        return;

    PlayerCharacter->StopInteractWithObject();
    ItemStaticMesh->AddImpulse(UseDirection * 20000);
}


// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	ItemStaticMesh->SetSimulatePhysics(true);
	ItemStaticMesh->SetEnableGravity(true);
	ItemStaticMesh->BodyInstance.bUseCCD = true;

	ItemStaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemStaticMesh->SetCollisionObjectType(ECC_WorldStatic);
	ItemStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemStaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ItemStaticMesh->SetNotifyRigidBodyCollision(true);

	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Sphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block); // Replace with your HighlightTrace channel
	Sphere->SetNotifyRigidBodyCollision(true);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

