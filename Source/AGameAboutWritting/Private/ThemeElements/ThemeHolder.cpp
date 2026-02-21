// Fill out your copyright notice in the Description page of Project Settings.


#include "ThemeElements/ThemeHolder.h"

// Sets default values
AThemeHolder::AThemeHolder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create arrow root
	ArrowRoot = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowRoot"));
	SetRootComponent(ArrowRoot);

	// Create mesh component
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(ArrowRoot);

	// Allow the mesh to be editable & movable in editor
	StaticMesh->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
void AThemeHolder::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* OverlayMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FXMaterials/OverlayFx/M_DestroyableObjectOverlayMat.M_DestroyableObjectOverlayMat"));
	StaticMesh->SetOverlayMaterial(UMaterialInstanceDynamic::Create(OverlayMat, this));
}

// Called every frame
void AThemeHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThemeHolder::OnHitByPlayerLaser_Implementation()
{
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(StaticMesh->GetOverlayMaterial())) {
		MID->SetScalarParameterValue(TEXT("HighlightStrenght"), 1.0f);
	}
}

void AThemeHolder::OnNoLongerHitByPlayerLaser_Implementation()
{
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(StaticMesh->GetOverlayMaterial())) {
		MID->SetScalarParameterValue(TEXT("HighlightStrenght"), 0.0f);
	}
}

void AThemeHolder::DetachFromHolder(AItemTheme* item_to_detach) {
	if (!item_to_detach) {
		return;
	}
	item_to_detach->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	TriggerThemeRemoved(item_to_detach);
	item_to_detach->Execute_StopInteract(item_to_detach,this);
}


void AThemeHolder::AttachToHolder(AItemTheme* Itemtheme) {
	Itemtheme->SetupItemAttachment();
	FAttachmentTransformRules Rules{ EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false };
	Itemtheme->AttachToComponent(StaticMesh, Rules, "HolderSocket");
}

void AThemeHolder::TriggerThemeInserted(AItemTheme* Theme)
{ 
	OnThemeInserted.Broadcast(Theme);
}

