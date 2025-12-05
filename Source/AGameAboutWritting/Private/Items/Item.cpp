// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Item.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = true;

    // --------------------------
    // NEW ROOT: AttachPivot
    // --------------------------
    AttachPivot = CreateDefaultSubobject<USceneComponent>(TEXT("AttachPivot"));
    SetRootComponent(AttachPivot);
    RootComponent = AttachPivot;

    AttachOffset = CreateDefaultSubobject<USceneComponent>(TEXT("AttachOffset"));
    AttachOffset->SetupAttachment(AttachPivot);

    // --------------------------
    // Static Mesh (child of Pivot)
    // --------------------------
    ItemStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemStaticMesh"));
    ItemStaticMesh->SetupAttachment(AttachPivot);
    ItemStaticMesh->BodyInstance.bUseCCD = true;
    ItemStaticMesh->SetSimulatePhysics(true);

    // --------------------------
    // Arrow (child of Pivot)
    // Designer sets THIS to define in-hand orientation!
    // --------------------------
    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->SetupAttachment(AttachPivot);

    // --------------------------
    // Interaction Sphere
    // --------------------------
    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetupAttachment(ItemStaticMesh);
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
    if (!ActorThatInteract) return false;

    // Try cast to player
    auto* PC = Cast<APlayerCharacter>(ActorThatInteract);
    if (!PC) return false;
    PlayerCharacter = PC;

    SetupItemAttachment();

// 3. Get arrow's current local rotation (inside the item)
    FRotator ArrowLocalRot = Arrow->GetRelativeRotation();

    // 5. Apply it
    // 2) Make the Arrow’s rotation zero in socket space
    const FQuat ArrowLocal = Arrow->GetRelativeRotation().Quaternion();
    const FQuat PivotLocal = ArrowLocal.Inverse();      // <- key line

    AttachPivot->SetRelativeRotation(PivotLocal.Rotator());    // ------------------------------------------------------
    // 3. Compute offset using RightHandSocket (per mesh)
    // ------------------------------------------------------
    FVector Offset = PivotLocal.RotateVector(-Arrow->GetRelativeLocation());
    AttachPivot->SetRelativeLocation(Offset);


    // ------------------------------------------------------
    // 4. Snap AttachPivot to hand socket
    // ------------------------------------------------------

    FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true);
    AttachPivot->AttachToComponent(
        PlayerCharacter->GetMesh(),
        Rules,
        TEXT("hand_rSocket")
    );


    // ------------------------------------------------------
    // 5. Player now holds it
    // ------------------------------------------------------
    PlayerCharacter->PlayHoldingAnimationMontage(true, IsOneHanded);

    IsHeld = true;
    return true;
}

void AItem::StopInteract_Implementation(AActor* ActorStopingInteract) {
    if (!ActorStopingInteract) return;

    Sphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

    // Re-enable physics
    ItemStaticMesh->SetSimulatePhysics(true);

    // Detach pivot from player
    FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, true);
    AttachPivot->DetachFromComponent(Rules);

    PlayerCharacter = Cast<APlayerCharacter>(ActorStopingInteract);
    if (PlayerCharacter)
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
    ItemStaticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ItemStaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    ItemStaticMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	ItemStaticMesh->SetNotifyRigidBodyCollision(true);

	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block); // Replace with your HighlightTrace channel
	Sphere->SetNotifyRigidBodyCollision(true);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FHandTransforms AItem::GetHandsSockets_Implementation() const
{
    FHandTransforms Result;
    Result.use_left_hand = ItemStaticMesh->DoesSocketExist("LeftHandSocket");
    Result.use_right_hand = true;

    if (Result.use_left_hand)
        Result.LeftHandTransform = ItemStaticMesh->GetSocketTransform("LeftHandSocket", RTS_World);

    Result.RightHandrelativeTransform = FTransform(
        Arrow->GetRelativeRotation().Quaternion(),
        AttachOffset->GetRelativeLocation(),
        FVector::OneVector
    );

    return Result;
}

void AItem::SetupItemAttachment() {
    Sphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

    ItemStaticMesh->SetSimulatePhysics(false);
    ItemStaticMesh->SetRelativeRotation(FRotator::ZeroRotator);
    ItemStaticMesh->SetRelativeLocation(FVector::ZeroVector);

    ItemStaticMesh->AttachToComponent(
        AttachPivot,
        FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true)
    );
}
