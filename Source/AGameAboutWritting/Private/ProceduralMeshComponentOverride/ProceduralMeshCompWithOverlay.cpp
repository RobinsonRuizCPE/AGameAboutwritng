// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMeshComponentOverride/ProceduralMeshCompWithOverlay.h"
#include "ProceduralMeshComponentOverride/FMyProcMeshSceneProxy.h"
// collisions
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

constexpr int32 TrianglesPerFrame = 5000;


UProceduralMeshCompWithOverlay::UProceduralMeshCompWithOverlay(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false; // we’ll enable only when needed
}

void UProceduralMeshCompWithOverlay::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!ActiveSplitTask) {
        return;
    }

    if (!ActiveSplitTask->bFinished)
    {
        ProcessSplitTask(TrianglesPerFrame);
    }

    if (ActiveSplitTask->bFinished && PendingSections.Num() == 0)
    {
        // Finished
        ActiveSplitTask.Reset();
        SetComponentTickEnabled(false);
        bMeshBuildInProgress = false;
        return;
    }

    LogBuildingStats();

    // Process 1 section per frame (you can change this to N per frame)
    FPendingSection S = PendingSections[0];
    PendingSections.RemoveAt(0);

    CreateMeshSection(
        S.SectionIndex,
        S.Vertices,
        S.Triangles,
        S.Normals,
        S.UV0,
        S.VertexColors,
        S.Tangents,
        false
    );

    if (S.Material)
        SetMaterial(S.SectionIndex, S.Material);
}

FPrimitiveSceneProxy* UProceduralMeshCompWithOverlay::CreateSceneProxy()
{
    return new FMyProcMeshSceneProxy(this);
}

void UProceduralMeshCompWithOverlay::CopyFrom(UProceduralMeshComponent* Other)
{
    if (!Other) return;

    ActiveSplitTask = MakeUnique<FMeshSplitTask>();
    ActiveSplitTask->MaxTrianglesPerChunk = TrianglesPerFrame;
    // Reserve to avoid reallocs
    int32 TotalSections = Other->GetNumSections();
    ActiveSplitTask->Sections.Reserve(TotalSections);
    ActiveSplitTask->Materials.Reserve(TotalSections);

    // Store pointers to A's sections
    for (int32 i = 0; i < Other->GetNumSections(); ++i)
    {
        ActiveSplitTask->Sections.Add(Other->GetProcMeshSection(i));
        ActiveSplitTask->Materials.Add(Other->GetMaterial(i));
    }

    SetComponentTickEnabled(true);
    CopyCollisionFrom(Other);
    bMeshBuildInProgress = true;
}

void UProceduralMeshCompWithOverlay::Merge(UProceduralMeshCompWithOverlay* OutMesh, UProceduralMeshCompWithOverlay* A, UProceduralMeshCompWithOverlay* B)
{
    if (!OutMesh || !A || !B)
        return;

    OutMesh->bUseComplexAsSimpleCollision = false;
    OutMesh->bUseAsyncCooking = true;

    OutMesh->ClearAllMeshSections();
    OutMesh->PendingSections.Empty();

    OutMesh->ConvexElementToCopy = A->ProcMeshBodySetup->AggGeom.ConvexElems;
    OutMesh->ConvexElementToCopy.Append(B->ProcMeshBodySetup->AggGeom.ConvexElems);

    PrepareMergedSections(OutMesh, A, B);
    OutMesh->CopyCollisionFrom(A);
    OutMesh->CopyCollisionFrom(B);
    OutMesh->bMeshBuildInProgress = true;
}

void UProceduralMeshCompWithOverlay::ProcessSplitTask(int32 TrianglesToProcess)
{
    auto& Task = *ActiveSplitTask;

    while (TrianglesToProcess > 0 && !Task.bFinished)
    {
        if (Task.CurrentSection >= Task.Sections.Num())
        {
            // Done!
            Task.bFinished = true;
            return;
        }

        const FProcMeshSection& Sec = *Task.Sections[Task.CurrentSection];
        int32 TotalTriangles = Sec.ProcIndexBuffer.Num() / 3;

        while (Task.CurrentTriangle < TotalTriangles && TrianglesToProcess > 0)
        {
            ProcessOneTriangleChunk(Sec, Task, TrianglesToProcess);
        }

        if (Task.CurrentTriangle >= TotalTriangles)
        {
            Task.CurrentTriangle = 0;
            Task.CurrentSection++;
        }
    }
}

void UProceduralMeshCompWithOverlay::ProcessOneTriangleChunk(
    const FProcMeshSection& Sec,
    FMeshSplitTask& Task,
    int32& TrianglesLeft)
{
    const int32 IndexCount = Sec.ProcIndexBuffer.Num();

    // No triangles in this section -> skip
    if (IndexCount < 3)
    {
        Task.CurrentTriangle = 0;
        Task.CurrentSection++;
        return;
    }

    const int32 TotalTriangles = IndexCount / 3;
    const int32 TriIndex = Task.CurrentTriangle;

    // If we reached past the end, stop processing this section
    if (TriIndex >= TotalTriangles)
    {
        Task.CurrentTriangle = 0;
        Task.CurrentSection++;
        return;
    }

    const int32 BaseIndex = TriIndex * 3;

    // SAFETY CHECK prevent out-of-range access
    if (BaseIndex + 2 >= IndexCount)
    {
        Task.CurrentTriangle = 0;
        Task.CurrentSection++;
        return;
    }

    // Start a new chunk if needed
    if (TriIndex == 0 ||
        PendingSections.Num() == 0 ||
        PendingSections.Last().Triangles.Num() / 3 >= Task.MaxTrianglesPerChunk)
    {
        FPendingSection NewChunk;
        NewChunk.SectionIndex = Task.CurrentChunkIndex++;
        NewChunk.Material = Task.Materials[Task.CurrentSection];
        NewChunk.bCollision = Sec.bEnableCollision;

        PendingSections.Add(MoveTemp(NewChunk));
    }

    FPendingSection& Chunk = PendingSections.Last();

    // Read indices safely
    uint32 i0 = Sec.ProcIndexBuffer[BaseIndex + 0];
    uint32 i1 = Sec.ProcIndexBuffer[BaseIndex + 1];
    uint32 i2 = Sec.ProcIndexBuffer[BaseIndex + 2];

    uint32 oldIndices[3] = { i0, i1, i2 };
    uint32 newIndices[3];

    for (int k = 0; k < 3; k++)
    {
        int32 old = (int32)oldIndices[k];
        int32* found = Chunk.IndexRemap.Find(old);

        if (!found)
        {
            int32 newIndex = Chunk.Vertices.Num();
            Chunk.IndexRemap.Add(old, newIndex);

            if (old < 0 || old >= Sec.ProcVertexBuffer.Num())
            {
                // Invalid mesh data: skip this triangle
                Task.CurrentTriangle++;
                TrianglesLeft--;
                return;
            }

            const FProcMeshVertex& V = Sec.ProcVertexBuffer[old];
            Chunk.Vertices.Add(V.Position);
            Chunk.Normals.Add(V.Normal);
            Chunk.UV0.Add(V.UV0);
            Chunk.VertexColors.Add(V.Color);
            Chunk.Tangents.Add(V.Tangent);

            newIndices[k] = newIndex;
        }
        else
        {
            newIndices[k] = *found;
        }
    }

    Chunk.Triangles.Add(newIndices[0]);
    Chunk.Triangles.Add(newIndices[1]);
    Chunk.Triangles.Add(newIndices[2]);

    Task.CurrentTriangle++;
    TrianglesLeft--;
}

void UProceduralMeshCompWithOverlay::PrepareMergedSections(
    UProceduralMeshCompWithOverlay* OutMesh,
    UProceduralMeshCompWithOverlay* A,
    UProceduralMeshCompWithOverlay* B)
{
    OutMesh->ActiveSplitTask = MakeUnique<FMeshSplitTask>();

    // Reserve to avoid reallocs
    int32 TotalSections = A->GetNumSections() + B->GetNumSections();
    OutMesh->ActiveSplitTask->Sections.Reserve(TotalSections);
    OutMesh->ActiveSplitTask->Materials.Reserve(TotalSections);

    // Store pointers to A's sections
    for (int32 i = 0; i < A->GetNumSections(); ++i)
    {
        OutMesh->ActiveSplitTask->Sections.Add(A->GetProcMeshSection(i));
        OutMesh->ActiveSplitTask->Materials.Add(A->GetMaterial(i));
    }

    // Store pointers to B's sections
    for (int32 i = 0; i < B->GetNumSections(); ++i)
    {
        OutMesh->ActiveSplitTask->Sections.Add(B->GetProcMeshSection(i));
        OutMesh->ActiveSplitTask->Materials.Add(B->GetMaterial(i));
    }

    OutMesh->SetComponentTickEnabled(true);
}


FBoxSphereBounds UProceduralMeshCompWithOverlay::ComputeBoundsFromConvex() const
{
    FBox Box(ForceInit);

    for (const FKConvexElem& Elem : ProcMeshBodySetup->AggGeom.ConvexElems)
    {
        for (const FVector& V : Elem.VertexData)
        {
            Box += V; // expand box to include vertex
        }
    }

    if (!Box.IsValid)
    {
        // fallback if no vertices
        return FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.f);
    }

    FBoxSphereBounds Result;
    Result.Origin = Box.GetCenter();
    Result.BoxExtent = Box.GetExtent();
    Result.SphereRadius = Result.BoxExtent.Size();

    return Result;
}

void UProceduralMeshCompWithOverlay::CopyCollisionFrom(UProceduralMeshComponent* Other)
{
    if (!Other->ProcMeshBodySetup)
        return;

    const TArray<FKConvexElem>& OtherConvexElems = Other->ProcMeshBodySetup->AggGeom.ConvexElems;
    for (const FKConvexElem& Elem : OtherConvexElems)
    {
        AddCollisionConvexMesh(Elem.VertexData);
    }

    // Copy general collision settings
    SetCollisionProfileName(Other->GetCollisionProfileName());
    SetCollisionEnabled(Other->GetCollisionEnabled());
}

void UProceduralMeshCompWithOverlay::LogBuildingStats() {
    int32 TotalTrianglesToSplit = 0;
    int32 TotalTrianglesProcessed = 0;

    // Screen message ID to update text in place
    int32 ScreenMessageId = 12345;
    if (GEngine)
    {
        FString Msg;

        if (ActiveSplitTask && !ActiveSplitTask->bFinished)
        {
            for (int32 s = 0; s < ActiveSplitTask->Sections.Num(); s++)
            {
                const FProcMeshSection* Sec = ActiveSplitTask->Sections[s];
                if (!Sec) continue;
                TotalTrianglesToSplit += Sec->ProcIndexBuffer.Num() / 3;
            }

            for (int32 s = 0; s < ActiveSplitTask->CurrentSection; s++)
            {
                const FProcMeshSection* Sec = ActiveSplitTask->Sections[s];
                if (!Sec) continue;
                TotalTrianglesProcessed += Sec->ProcIndexBuffer.Num() / 3;
            }

            // Add triangles processed in the current section
            TotalTrianglesProcessed += ActiveSplitTask->CurrentTriangle;

            Msg = FString::Printf(
                TEXT("[Merging Meshes] Splitting triangles: %d / %d (%.1f%%)"),
                TotalTrianglesProcessed,
                TotalTrianglesToSplit,
                TotalTrianglesToSplit > 0 ? (100.f * TotalTrianglesProcessed / TotalTrianglesToSplit) : 0.f
            );
            GEngine->AddOnScreenDebugMessage(ScreenMessageId, 0.f, FColor::Yellow, Msg);

        }
        if (PendingSections.Num() != 0)
        {
            // Compute original total using section indices (0..N-1)
            int32 TotalSectionsToBuild = 0;
            for (const FPendingSection& Sec : PendingSections)
            {
                TotalSectionsToBuild = FMath::Max(TotalSectionsToBuild, Sec.SectionIndex + 1);
            }

            int32 BuiltSections = TotalSectionsToBuild - PendingSections.Num();

            Msg = FString::Printf(
                TEXT("[Merging Meshes] Building sections: %d / %d (%.1f%%)"),
                BuiltSections,
                TotalSectionsToBuild,
                TotalSectionsToBuild > 0 ? (100.f * BuiltSections / TotalSectionsToBuild) : 0.f
            );
            GEngine->AddOnScreenDebugMessage(ScreenMessageId +1, 0.f, FColor::Yellow, Msg);

        }
    }
}
