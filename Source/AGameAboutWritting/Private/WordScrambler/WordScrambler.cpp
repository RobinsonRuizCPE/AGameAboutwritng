// Fill out your copyright notice in the Description page of Project Settings.


#include "WordScrambler/WordScrambler.h"

//Mesh merging
#include "StaticMeshDescription.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshOperations.h"
#include "Engine/StaticMesh.h"

// collisions
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

AWordScrambler::AWordScrambler() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create mesh component
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(ArrowRoot);

	// Allow the mesh to be editable & movable in editor
	SkeletalMesh->SetMobility(EComponentMobility::Movable);
}


AItemTheme* AWordScrambler::ScrambleItems(AItemTheme* first_item, AItemTheme* second_item) {
    if (!first_item || !second_item) return nullptr;

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // 2. Create new item
    AItemTheme* NewItem = World->SpawnActor<AItemTheme>(AItemTheme::StaticClass());
    UProceduralMeshCompWithOverlay::Merge(NewItem->ProceduralMesh, first_item->ProceduralMesh, second_item->ProceduralMesh);
    MergeItemsThemes(first_item, second_item, NewItem);

    // 7. Destroy originals
    first_item->Destroy();
    second_item->Destroy();

    return NewItem;
}

void AWordScrambler::MergeItemsThemes(AItemTheme* first_item, AItemTheme* second_item, AItemTheme* out_item) {
    out_item->GetThemeSet() = first_item->GetThemeSet();
    out_item->GetThemeSet().Append(second_item->GetThemeSet());

    // 5. Merge weightmaps
    for (const auto& Pair : first_item->GetThemeRelatedList())
        out_item->GetThemeRelatedList().FindOrAdd(Pair.Key) += Pair.Value;

    for (const auto& Pair : second_item->GetThemeRelatedList())
        out_item->GetThemeRelatedList().FindOrAdd(Pair.Key) += Pair.Value;
}
