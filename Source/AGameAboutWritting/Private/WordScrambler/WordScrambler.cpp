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

    // 1. Merge mesh
    UStaticMesh* MergedMesh = MergeMeshes(
        first_item->GetMeshReference(),
        second_item->GetMeshReference(),
        this
    );

    MergeMaterials(first_item->GetMeshReference(), second_item->GetMeshReference(), MergedMesh);
    CopySimpleCollision(first_item->GetMeshReference(), FTransform::Identity, second_item->GetMeshReference(), FTransform::Identity, MergedMesh);

    // 2. Create new item
    AItemTheme* NewItem = World->SpawnActor<AItemTheme>(AItemTheme::StaticClass());
    NewItem->ItemStaticMesh->SetStaticMesh(MergedMesh);

    MergeItemsThemes(first_item, second_item, NewItem);

    // 7. Destroy originals
    first_item->Destroy();
    second_item->Destroy();

    return NewItem;
}

UStaticMesh* AWordScrambler::MergeMeshes(UStaticMesh* MeshA, UStaticMesh* MeshB, UObject* Outer)
{
    if (!MeshA || !MeshB) return nullptr;

    // Create the new static mesh
    UStaticMesh* NewMesh = NewObject<UStaticMesh>(Outer);
    NewMesh->SetNumSourceModels(1);

    // Create an empty mesh description inside the new mesh
    FMeshDescription* NewDesc = NewMesh->CreateMeshDescription(0);

    // Copy Mesh A into NewDesc
    const FMeshDescription* DescA = MeshA->GetMeshDescription(0);
    *NewDesc = *DescA;

    // Copy Mesh B
    FMeshDescription BCopy = *MeshB->GetMeshDescription(0);

    // Append mesh B onto mesh A
    FStaticMeshOperations::FAppendSettings setting{};
    FStaticMeshOperations::AppendMeshDescription(BCopy, *NewDesc, setting);

    // Save description back into the static mesh
    NewMesh->CommitMeshDescription(0);

    // Build render data
    NewMesh->Build(false);
    NewMesh->PostEditChange();

    return NewMesh;
}

void AWordScrambler::MergeMaterials(UStaticMesh* MeshA, UStaticMesh* MeshB, UStaticMesh* out_mesh) {
    if (!MeshA || !MeshB || !out_mesh)
        return;

    const TArray<FStaticMaterial>& MatsA = MeshA->GetStaticMaterials();
    const TArray<FStaticMaterial>& MatsB = MeshB->GetStaticMaterials();

    const int32 NumA = MatsA.Num();
    const int32 NumB = MatsB.Num();
    const int32 NumMerged = NumA + NumB;

    if (NumA == 0 && NumB == 0)
        return;

    // Ensure output mesh has room
    TArray<FStaticMaterial>& OutMats = out_mesh->GetStaticMaterials();
    OutMats.SetNum(NumMerged);

    // --- 1. Fill slots that belonged to A with B's materials (loop B)
    for (int32 i = 0; i < NumA; i++)
    {
        int32 BIndex = i % NumB;   // loop through B materials
        OutMats[i] = MatsB[BIndex];
    }

    // --- 2. Fill slots that belonged to B with A's materials (loop A)
    for (int32 j = 0; j < NumB; j++)
    {
        int32 OutIndex = NumA + j;
        int32 AIndex = j % NumA;   // loop through A materials
        OutMats[OutIndex] = MatsA[AIndex];
    }

    // Notify Unreal
    out_mesh->PostEditChange();
    out_mesh->MarkPackageDirty();
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

void AWordScrambler::CopySimpleCollision(
    UStaticMesh* MeshA, const FTransform& TransformA,
    UStaticMesh* MeshB, const FTransform& TransformB,
    UStaticMesh* OutMesh)
{
    // Create body setup for the merged mesh
    OutMesh->CreateBodySetup();
    UBodySetup* OutBS = OutMesh->GetBodySetup();
    OutBS->AggGeom = FKAggregateGeom();

    auto CopyFrom = [&](UStaticMesh* SM, const FTransform& T)
    {
        UBodySetup* BS = SM->GetBodySetup();
        if (!BS) return;

        // Convex collision
        for (const FKConvexElem& Convex : BS->AggGeom.ConvexElems)
        {
            FKConvexElem Copy = Convex;
            for (FVector& V : Copy.VertexData)
            {
                V = T.TransformPosition(V);
            }
            Copy.UpdateElemBox();
            OutBS->AggGeom.ConvexElems.Add(Copy);
        }

        // Box collision
        for (const FKBoxElem& Box : BS->AggGeom.BoxElems)
        {
            FKBoxElem Copy = Box;
            Copy.Center = T.TransformPosition(Box.Center);
            Copy.Rotation = (T.GetRotation() * FQuat(Box.Rotation)).Rotator();
            OutBS->AggGeom.BoxElems.Add(Copy);
        }

        // Sphere collision
        for (const FKSphereElem& Sphere : BS->AggGeom.SphereElems)
        {
            FKSphereElem Copy = Sphere;
            Copy.Center = T.TransformPosition(Sphere.Center);
            OutBS->AggGeom.SphereElems.Add(Copy);
        }

        // Capsule collision
        for (const FKSphylElem& Cap : BS->AggGeom.SphylElems)
        {
            FKSphylElem Copy = Cap;
            Copy.Center = T.TransformPosition(Cap.Center);
            Copy.Rotation = (T.GetRotation() * FQuat(Cap.Rotation)).Rotator();
            OutBS->AggGeom.SphylElems.Add(Copy);
        }
    };

    // Copy both simple collisions
    CopyFrom(MeshA, TransformA);
    CopyFrom(MeshB, TransformB);

    // Only use simple collision (important!)
    OutBS->bHasCookedCollisionData = false;
    OutBS->InvalidatePhysicsData();
    OutBS->CreatePhysicsMeshes();
    OutBS->CollisionTraceFlag = CTF_UseSimpleAsComplex;

    // Re-init resources
    OutMesh->InitResources();
}