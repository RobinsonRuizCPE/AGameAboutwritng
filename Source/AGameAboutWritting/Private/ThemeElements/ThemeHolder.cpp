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
	
}

// Called every frame
void AThemeHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

