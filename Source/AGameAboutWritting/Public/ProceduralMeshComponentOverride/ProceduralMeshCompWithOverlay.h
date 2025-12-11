// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "TimerManager.h"
#include "PhysicsEngine/ConvexElem.h"

#include "ProceduralMeshCompWithOverlay.generated.h"


UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent))
class AGAMEABOUTWRITTING_API UProceduralMeshCompWithOverlay : public UProceduralMeshComponent
{
	GENERATED_BODY()

    struct FPendingSection
    {
        int32 SectionIndex;
        TArray<FVector> Vertices;
        TArray<int32> Triangles;
        TArray<FVector> Normals;
        TArray<FVector2D> UV0;
        TArray<FColor> VertexColors;
        TArray<FProcMeshTangent> Tangents;
        UMaterialInterface* Material = nullptr;
        bool bCollision = false;

        TMap<int32, int32> IndexRemap;
    };

    struct FMeshSplitTask
    {
        TArray<const FProcMeshSection*> Sections;
        TArray<UMaterialInterface*> Materials;
        int32 CurrentSection = 0;
        int32 CurrentTriangle = 0;
        int32 MaxTrianglesPerChunk = 5000;
        int32 CurrentChunkIndex = 0;
        bool bFinished = false;
    };
	
public:

	UProceduralMeshCompWithOverlay(const FObjectInitializer& ObjectInitializer);

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	static void Merge(UProceduralMeshCompWithOverlay* OutMesh, UProceduralMeshCompWithOverlay* First, UProceduralMeshCompWithOverlay* Second);

	void CopyFrom(UProceduralMeshComponent* Other);

    bool isMeshBuildInProgress() const { return bMeshBuildInProgress; }

    FBoxSphereBounds ComputeBoundsFromConvex() const;


protected:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

private:
    void ProcessSplitTask(int32 TrianglesToProcess);
    void ProcessOneTriangleChunk(const FProcMeshSection& Sec, FMeshSplitTask& Task, int32& TrianglesLeft);
    static void SplitSectionByTriangles(const FProcMeshSection& Section, UMaterialInterface* Material, int32 MaxTrianglesPerChunk, TArray<FPendingSection>& OutChunks);
    static void PrepareMergedSections(UProceduralMeshCompWithOverlay* OutMesh, UProceduralMeshCompWithOverlay* A, UProceduralMeshCompWithOverlay* B);
    void LogBuildingStats();

protected:
    TArray<FKConvexElem> ConvexElementToCopy;

private:
	void CopyCollisionFrom(UProceduralMeshComponent* Other);
    bool bBuildSectionsInTick = false;
    TArray<FPendingSection> PendingSections;
    TUniquePtr<FMeshSplitTask> ActiveSplitTask;
    FTimerHandle SectionBuildTimer;
    bool bMeshBuildInProgress = false;

};
