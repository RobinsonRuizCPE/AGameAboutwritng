// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMeshComponentOverride/FMyProcMeshSceneProxy.h"
#include "ProceduralMeshComponentOverride/ProceduralMeshCompWithOverlay.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "SceneManagement.h"
#include "RenderingThread.h"
#include "LocalVertexFactory.h"
#include "MaterialDomain.h"
#include "PhysicsEngine/BodySetup.h"
#include "RayTracingInstance.h"
#include "Engine/Engine.h"

///////////////////////////////////////////////////////////
// Scene Proxy

/** Class representing a single section of the proc mesh */


static TAutoConsoleVariable<int32> CVarRayTracingProceduralMeshWithOverlay(
	TEXT("r.RayTracing.Geometry.ProceduralMeshes"),
	1,
	TEXT("Include procedural meshes in ray tracing effects (default = 1 (procedural meshes enabled in ray tracing))"));

static void ConvertProcMeshWithoverlayToDynMeshVertex(FDynamicMeshVertex& Vert, const FProcMeshVertex& ProcVert)
{
	Vert.Position = (FVector3f)ProcVert.Position;
	Vert.Color = ProcVert.Color;
	Vert.TextureCoordinate[0] = FVector2f(ProcVert.UV0);	// LWC_TODO: Precision loss
	Vert.TextureCoordinate[1] = FVector2f(ProcVert.UV1);	// LWC_TODO: Precision loss
	Vert.TextureCoordinate[2] = FVector2f(ProcVert.UV2);	// LWC_TODO: Precision loss
	Vert.TextureCoordinate[3] = FVector2f(ProcVert.UV3);	// LWC_TODO: Precision loss
	Vert.TangentX = ProcVert.Tangent.TangentX;
	Vert.TangentZ = ProcVert.Normal;
	Vert.TangentZ.Vector.W = ProcVert.Tangent.bFlipTangentY ? -127 : 127;
}

FMyProcMeshSceneProxy::FMyProcMeshSceneProxy(UProceduralMeshCompWithOverlay* Component)
	: FPrimitiveSceneProxy(Component)
	, BodySetup(Component->GetBodySetup())
	, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	, OverlayMaterial(Component->OverlayMaterial)
{
	// Copy each section
	const int32 NumSections = Component->GetNumSections();
	Sections.AddZeroed(NumSections);
	for (int SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
	{
		FProcMeshSection* SrcSection = Component->GetProcMeshSection(SectionIdx);
		if (SrcSection->ProcIndexBuffer.Num() > 0 && SrcSection->ProcVertexBuffer.Num() > 0)
		{
			FProcMeshWithoverlayProxySection* NewSection = new FProcMeshWithoverlayProxySection(GetScene().GetFeatureLevel());

			// Copy data from vertex buffer
			const int32 NumVerts = SrcSection->ProcVertexBuffer.Num();

			// Allocate verts

			TArray<FDynamicMeshVertex> Vertices;
			Vertices.SetNumUninitialized(NumVerts);
			// Copy verts
			for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
			{
				const FProcMeshVertex& ProcVert = SrcSection->ProcVertexBuffer[VertIdx];
				FDynamicMeshVertex& Vert = Vertices[VertIdx];
				ConvertProcMeshWithoverlayToDynMeshVertex(Vert, ProcVert);
			}

			// Copy index buffer
			NewSection->IndexBuffer.Indices = SrcSection->ProcIndexBuffer;

			NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, Vertices, 4);

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->VertexFactory);

			// Grab material
			NewSection->Material = Component->GetMaterial(SectionIdx);
			if (NewSection->Material == NULL)
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
			}

			// Copy visibility info
			NewSection->bSectionVisible = SrcSection->bSectionVisible;

			// Save ref to new section
			Sections[SectionIdx] = NewSection;

#if RHI_RAYTRACING
			if (IsRayTracingEnabled())
			{
				ENQUEUE_RENDER_COMMAND(InitProceduralMeshRayTracingGeometry)(
					[this, DebugName = Component->GetFName(), NewSection](FRHICommandListImmediate& RHICmdList)
					{
						FRayTracingGeometryInitializer Initializer;
				Initializer.DebugName = DebugName;
				Initializer.IndexBuffer = NewSection->IndexBuffer.IndexBufferRHI;
				Initializer.TotalPrimitiveCount = NewSection->IndexBuffer.Indices.Num() / 3;
				Initializer.GeometryType = RTGT_Triangles;
				Initializer.bFastBuild = true;
				Initializer.bAllowUpdate = false;

				FRayTracingGeometrySegment Segment;
				Segment.VertexBuffer = NewSection->VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
				Segment.MaxVertices = NewSection->VertexBuffers.PositionVertexBuffer.GetNumVertices();
				Segment.NumPrimitives = Initializer.TotalPrimitiveCount;

				Initializer.Segments.Add(Segment);

				NewSection->RayTracingGeometry.SetInitializer(Initializer);
				NewSection->RayTracingGeometry.InitResource(RHICmdList);
					});
			}
#endif
		}
	}
}

FMyProcMeshSceneProxy::~FMyProcMeshSceneProxy()
{
	for (FProcMeshWithoverlayProxySection* Section : Sections)
	{
		if (Section != nullptr)
		{
			Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();

#if RHI_RAYTRACING
			if (IsRayTracingEnabled())
			{
				Section->RayTracingGeometry.ReleaseResource();
			}
#endif

			delete Section;
		}
	}
}

/** Called on render thread to assign new dynamic data */
void FMyProcMeshSceneProxy::UpdateSection_RenderThread(FRHICommandListBase& RHICmdList, FProcMeshWithOverlaySectionUpdateData* SectionData)
{
	// Check we have data 
	if (SectionData != nullptr)
	{
		// Check it references a valid section
		if (SectionData->TargetSection < Sections.Num() &&
			Sections[SectionData->TargetSection] != nullptr)
		{
			FProcMeshWithoverlayProxySection* Section = Sections[SectionData->TargetSection];

			// Lock vertex buffer
			const int32 NumVerts = SectionData->NewVertexBuffer.Num();

			// Iterate through vertex data, copying in new info
			for (int32 i = 0; i < NumVerts; i++)
			{
				const FProcMeshVertex& ProcVert = SectionData->NewVertexBuffer[i];
				FDynamicMeshVertex Vertex;
				ConvertProcMeshWithoverlayToDynMeshVertex(Vertex, ProcVert);

				Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, Vertex.TextureCoordinate[1]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, Vertex.TextureCoordinate[2]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, Vertex.TextureCoordinate[3]);
				Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
			}

			{
				auto& VertexBuffer = Section->VertexBuffers.PositionVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = Section->VertexBuffers.ColorVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
				RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
				RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
			}

#if RHI_RAYTRACING
			if (IsRayTracingEnabled())
			{
				Section->RayTracingGeometry.ReleaseResource();

				FRayTracingGeometryInitializer Initializer;
				Initializer.IndexBuffer = Section->IndexBuffer.IndexBufferRHI;
				Initializer.TotalPrimitiveCount = Section->IndexBuffer.Indices.Num() / 3;
				Initializer.GeometryType = RTGT_Triangles;
				Initializer.bFastBuild = true;
				Initializer.bAllowUpdate = false;

				FRayTracingGeometrySegment Segment;
				Segment.VertexBuffer = Section->VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
				Segment.MaxVertices = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices();
				Segment.NumPrimitives = Initializer.TotalPrimitiveCount;

				Initializer.Segments.Add(Segment);

				Section->RayTracingGeometry.SetInitializer(Initializer);
				Section->RayTracingGeometry.InitResource(RHICmdList);
			}
#endif
		}

		// Free data sent from game thread
		delete SectionData;
	}
}

void FMyProcMeshSceneProxy::SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility)
{
	check(IsInRenderingThread());

	if (SectionIndex < Sections.Num() &&
		Sections[SectionIndex] != nullptr)
	{
		Sections[SectionIndex]->bSectionVisible = bNewVisibility;
	}
}

void FMyProcMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	// Set up wireframe material (if needed)
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy* WireframeMaterialInstance = NULL;
	if (bWireframe)
	{
		WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
	}

	// Iterate over sections
	for (const FProcMeshWithoverlayProxySection* Section : Sections)
	{
		if (Section != nullptr && Section->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &Section->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;

					bool bHasPrecomputedVolumetricLightmap;
					FMatrix PreviousLocalToWorld;
					int32 SingleCaptureIndex;
					bool bOutputVelocity;
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
					bOutputVelocity |= AlwaysHasVelocity();

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), ReceivesDecals(), bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);

					if (OverlayMaterial)
					{
						FMeshBatch& MeshOverlay = Collector.AllocateMesh();
						MeshOverlay.VertexFactory = &Section->VertexFactory;
						MeshOverlay.MaterialRenderProxy = OverlayMaterial->GetRenderProxy();
						MeshOverlay.Type = PT_TriangleList;
						MeshOverlay.ReverseCulling = IsLocalToWorldDeterminantNegative();

						FMeshBatchElement& Elem = MeshOverlay.Elements[0];
						Elem.IndexBuffer = &Section->IndexBuffer;
						Elem.FirstIndex = 0;
						Elem.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
						Elem.MinVertexIndex = 0;
						Elem.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

						Collector.AddMesh(ViewIndex, MeshOverlay);
					}
				}
			}
		}
	}

	// Draw bounds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			// Draw simple collision as wireframe if 'show collision', and collision is enabled, and we are not using the complex as the simple
			if (ViewFamily.EngineShowFlags.Collision && IsCollisionEnabled() && BodySetup->GetCollisionTraceFlag() != ECollisionTraceFlag::CTF_UseComplexAsSimple)
			{
				FTransform GeomTransform(GetLocalToWorld());
				BodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(FColor(157, 149, 223, 255), IsSelected(), IsHovered()).ToFColor(true), NULL, false, false, AlwaysHasVelocity(), ViewIndex, Collector);
			}

			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif
}

FPrimitiveViewRelevance FMyProcMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
	return Result;
}

bool FMyProcMeshSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FMyProcMeshSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}

uint32 FMyProcMeshSceneProxy::GetAllocatedSize(void) const
{
	return(FPrimitiveSceneProxy::GetAllocatedSize());
}


#if RHI_RAYTRACING


void FMyProcMeshSceneProxy::GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances)
{
	if (!CVarRayTracingProceduralMeshWithOverlay.GetValueOnRenderThread())
	{
		return;
	}

	for (int32 SegmentIndex = 0; SegmentIndex < Sections.Num(); ++SegmentIndex)
	{
		const FProcMeshWithoverlayProxySection* Section = Sections[SegmentIndex];
		if (Section != nullptr && Section->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = Section->Material->GetRenderProxy();

			if (Section->RayTracingGeometry.RayTracingGeometryRHI.IsValid())
			{
				check(Section->RayTracingGeometry.Initializer.IndexBuffer.IsValid());

				FRayTracingInstance RayTracingInstance;
				RayTracingInstance.Geometry = &Section->RayTracingGeometry;
				RayTracingInstance.InstanceTransforms.Add(GetLocalToWorld());

				uint32 SectionIdx = 0;
				FMeshBatch MeshBatch;

				MeshBatch.VertexFactory = &Section->VertexFactory;
				MeshBatch.SegmentIndex = 0;
				MeshBatch.MaterialRenderProxy = Section->Material->GetRenderProxy();
				MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
				MeshBatch.Type = PT_TriangleList;
				MeshBatch.DepthPriorityGroup = SDPG_World;
				MeshBatch.bCanApplyViewModeOverrides = false;
				MeshBatch.CastRayTracedShadow = IsShadowCast(Context.ReferenceView);

				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				BatchElement.IndexBuffer = &Section->IndexBuffer;

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
				bool bOutputVelocity;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
				bOutputVelocity |= AlwaysHasVelocity();

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Context.RayTracingMeshResourceCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(Context.RHICmdList, GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), ReceivesDecals(), bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

				RayTracingInstance.Materials.Add(MeshBatch);
				OutRayTracingInstances.Add(RayTracingInstance);
			}
		}
	}
}

#endif
