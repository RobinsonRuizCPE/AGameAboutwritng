// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewSceneUI/StaticMeshPreviewRenderer.h"

#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/DirectionalLightComponent.h"
#include "WrittingReviewGameInstance.h"

static float GPreviewSpawnZ = -10000.0f;
static const float GPreviewZStep = -1000.0f;

AItemPreviewActor::AItemPreviewActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    MeshComponent->SetupAttachment(Root);

    Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("PreviewLight"));
    Light->SetInnerConeAngle(30.f);
    Light->SetOuterConeAngle(45.f);
    Light->SetIntensity(8000.f);
    Light->bAffectsWorld = true;
    Light->LightingChannels.bChannel0 = false;
    Light->LightingChannels.bChannel1 = true;
    Light->SetMobility(EComponentMobility::Movable);
    Light->SetupAttachment(RootComponent);
    Light->LightingChannels.bChannel0 = false;
    Light->LightingChannels.bChannel1 = true;
    Light->SetIntensity(1000.f);
    Light->SetCastShadows(true);

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
    SceneCapture->SetupAttachment(Root);
    SceneCapture->SetRelativeLocation(FVector(150.f, 0.f, 0.f));
    SceneCapture->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));

    // Isolate capture
    SceneCapture->bCaptureEveryFrame = false;
    SceneCapture->bCaptureOnMovement = false;
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    SceneCapture->ShowFlags.SetAtmosphere(false);
    SceneCapture->ShowFlags.SetSkyLighting(false);
    SceneCapture->ShowFlags.SetPostProcessing(false);
    SceneCapture->ShowFlags.SetFog(false);
    SceneCapture->ShowFlags.SetDirectionalLights(true);
    SceneCapture->ShowFlags.SetPointLights(true);
    SceneCapture->ShowFlags.SetSpotLights(true);
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
    //SceneCapture->ShowFlags.Lighting = false;
    SceneCapture->ShowFlags.PostProcessing = false;
    SceneCapture->ShowFlags.Materials = true;
    SceneCapture->ShowOnlyActorComponents(this);

    // New stuff
    SceneCapture->ShowFlags.Lighting = true;
    SceneCapture->ShowFlags.DynamicShadows = true;


    // Collision/lighting isolation
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetCastShadow(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    // Mesh
    MeshComponent->LightingChannels.bChannel0 = false;
    MeshComponent->LightingChannels.bChannel1 = true;

    // ---- AUTO POSITION BELOW WORLD ----
    SetActorLocation(FVector(0.f, 0.f, GPreviewSpawnZ));

    // Increment for the next preview actor
    GPreviewSpawnZ += GPreviewZStep;
}

void AItemPreviewActor::InitializePreview(UStaticMesh* Mesh, int32 Width, int32 Height)
{
    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->RenderTargetFormat = RTF_RGBA8;
    RenderTarget->InitAutoFormat(Width, Height);
    RenderTarget->ClearColor = FLinearColor::Transparent;
    RenderTarget->UpdateResourceImmediate(true);

    SceneCapture->TextureTarget = RenderTarget;
    SceneCapture->CaptureScene();

    SetMesh(Mesh);
}

void AItemPreviewActor::SetMesh(UStaticMesh* Mesh)
{
    if (!Mesh) return;

    MeshComponent->SetStaticMesh(Mesh);

    // Fit camera distance dynamically
    const FBoxSphereBounds Bounds = MeshComponent->CalcBounds(FTransform::Identity);
    const float Distance = Bounds.SphereRadius * 2.0f;

    SceneCapture->SetRelativeLocation(FVector(Distance, 0.f, 0.f));
    CenterMeshInCapture();
    // Capture once after setup
    SceneCapture->CaptureScene();
}

void AItemPreviewActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bRotate && MeshComponent)
    {
        FRotator Rot = MeshComponent->GetRelativeRotation();
        Rot.Yaw += RotationSpeed * DeltaSeconds;
        MeshComponent->SetRelativeRotation(Rot);
        SceneCapture->CaptureScene();
    }
}

void AItemPreviewActor::CenterMeshInCapture()
{
    if (!MeshComponent || !MeshComponent->GetStaticMesh() || !SceneCapture)
        return;

    const FBoxSphereBounds LocalBounds = MeshComponent->CalcBounds(FTransform::Identity);
    const FVector OriginOffset = -LocalBounds.Origin;

    const FBoxSphereBounds MeshBounds = MeshComponent->GetStaticMesh()->GetBounds();
    const FVector Extent = MeshBounds.BoxExtent;

    const float Diagonal = Extent.GetAbsMax() * 2; 

    const float HalfFOVRadians = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
    float Distance = Diagonal / FMath::Tan(HalfFOVRadians);
    Distance *= 1.2f; // Add 20% padding for safety

    MeshComponent->SetRelativeLocation(OriginOffset);
    MeshComponent->SetRelativeRotation(FRotator::ZeroRotator);

    SceneCapture->SetRelativeLocation(FVector(Distance, 0.f, 0.f));
    SceneCapture->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));

    Light->SetRelativeLocation(FVector(Distance, 0.f, 0.f));
    Light->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));

    SceneCapture->CaptureScene();
}

UItemPreviewManager* UItemPreviewManager::Get(UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    UWorld* World = WorldContext->GetWorld();
    if (!World) return nullptr;
    UWrittingReviewGameInstance* GI = World->GetGameInstance<UWrittingReviewGameInstance>();
    return GI ? GI->GetPreviewManager() : nullptr;
}

UTextureRenderTarget2D* UItemPreviewManager::GetOrCreateRenderTargetFromMesh(UStaticMesh* Mesh, int32 Width, int32 Height) {
    if (auto found_preview_actor = PreviewActorsMap.FindRef(Mesh)) {
        return found_preview_actor->GetRenderTarget();
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AItemPreviewActor* NewPreview = GetWorld()->SpawnActor<AItemPreviewActor>(
        AItemPreviewActor::StaticClass(),
        FTransform::Identity,
        Params
        );

    if (!NewPreview)
    {
        return nullptr;
    }

    NewPreview->InitializePreview(Mesh, Width, Height);
    PreviewActorsMap.Add(Mesh, NewPreview);
    return NewPreview->GetRenderTarget();
}
