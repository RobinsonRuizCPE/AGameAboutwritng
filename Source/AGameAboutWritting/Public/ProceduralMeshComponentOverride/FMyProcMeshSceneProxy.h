// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "ProceduralMeshComponent.h"

/**
 * 
 */

class UProceduralMeshCompWithOverlay;

class FProcMeshWithoverlayProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FDynamicMeshIndexBuffer32 IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible;

#if RHI_RAYTRACING
	FRayTracingGeometry RayTracingGeometry;
#endif

	FProcMeshWithoverlayProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(NULL)
		, VertexFactory(InFeatureLevel, "FProcMeshWithoverlayProxySection")
		, bSectionVisible(true)
	{}
};

class FProcMeshWithOverlaySectionUpdateData
{
public:
	/** Section to update */
	int32 TargetSection;
	/** New vertex information */
	TArray<FProcMeshVertex> NewVertexBuffer;
};

class AGAMEABOUTWRITTING_API FMyProcMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	FMyProcMeshSceneProxy(UProceduralMeshCompWithOverlay* Component);
	~FMyProcMeshSceneProxy();

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	void UpdateSection_RenderThread(FRHICommandListBase& RHICmdList, FProcMeshWithOverlaySectionUpdateData* SectionData);

	void SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility);

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const;

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

#if RHI_RAYTRACING
	virtual bool IsRayTracingRelevant() const override { return true; }
	virtual bool HasRayTracingRepresentation() const override { return true; }
	virtual void GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances) override final;
#endif

private:
	TArray<FProcMeshWithoverlayProxySection*> Sections;
	UBodySetup* BodySetup;
	FMaterialRelevance MaterialRelevance;
	UMaterialInterface* OverlayMaterial;
};
