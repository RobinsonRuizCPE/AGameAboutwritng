// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewSceneUI/StaticMeshPreviewRenderer.h"
#include "WrittingReviewGameInstance.h"
#include "ImageUtils.h"
#include "RenderingThread.h"
#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/DirectionalLightComponent.h"


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
    Light->bAffectsWorld = true;
    Light->LightingChannels.bChannel0 = false;
    Light->LightingChannels.bChannel1 = true;
    Light->SetMobility(EComponentMobility::Movable);
    Light->SetupAttachment(RootComponent);
    Light->LightingChannels.bChannel0 = false;
    Light->LightingChannels.bChannel1 = true;
    Light->SetIntensity(1000.f);
    Light->SetCastShadows(false);

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
    SceneCapture->SetupAttachment(Root);
    SceneCapture->SetRelativeLocation(FVector(150.f, 0.f, 0.f));
    SceneCapture->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));

    // Isolate capture
    SceneCapture->bCaptureEveryFrame = false;
    SceneCapture->bCaptureOnMovement = false;
    SceneCapture->bUseRayTracingIfEnabled = false;
    SceneCapture->ShowFlags.SetAtmosphere(false);
    SceneCapture->ShowFlags.SetSkyLighting(false);
    SceneCapture->ShowFlags.SetPostProcessing(false);
    SceneCapture->ShowFlags.SetFog(false);
    SceneCapture->ShowFlags.SetSpotLights(true);
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
    SceneCapture->ShowFlags.PostProcessing = false;
    SceneCapture->ShowFlags.Materials = true;
    SceneCapture->ShowOnlyActorComponents(this);
    SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    SceneCapture->ShowOnlyComponent(MeshComponent);
    SceneCapture->ShowFlags.Translucency = true;

    SceneCapture->ShowFlags.SetDeferredLighting(true);
    SceneCapture->ShowFlags.SetLighting(true);
    SceneCapture->ShowFlags.SetNaniteMeshes(false);

    // New stuff
    SceneCapture->ShowFlags.Lighting = true;
    SceneCapture->ShowFlags.DynamicShadows = false;

    // Collision/lighting isolation
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetCastShadow(false);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    // Mesh
    MeshComponent->LightingChannels.bChannel0 = false;
    MeshComponent->LightingChannels.bChannel1 = true;

    // ---- AUTO POSITION BELOW WORLD ----
    SetActorLocation(FVector(0.f, 0.f, GPreviewSpawnZ));

    // Increment for the next preview actor
    GPreviewSpawnZ += GPreviewZStep;

    CopyTextureMaterial = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Game/Items/M_RenderTargetCopy.M_RenderTargetCopy")
    );
}

void AItemPreviewActor::InitializePreview(UStaticMesh* Mesh, int32 Width, int32 Height)
{
    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->RenderTargetFormat = RTF_RGBA8;
    RenderTarget->InitAutoFormat(Width, Height);
    RenderTarget->ClearColor = FLinearColor::Transparent;
    RenderTarget->UpdateResourceImmediate(true);

    SceneCapture->TextureTarget = RenderTarget;

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
    StartSpriteSheetCapture();
}

void AItemPreviewActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsCapturingSpriteSheet && MeshComponent)
    {
        float DegreesPerFrame = 360.f / NumFramesToCapture;

        // rotate exactly per frame
        MeshComponent->AddRelativeRotation(FRotator(0, DegreesPerFrame, 0));

        CaptureFrameToSpriteSheet();

        FramesCaptured++;

        if (FramesCaptured >= NumFramesToCapture)
        {
            bIsCapturingSpriteSheet = false;
            bRotate = false;

            SaveSpriteSheetToFile();
        }
    }

    if (shouldAnimateSheet)
    {
        float TimePerFrame = 1.0f / 30.0f; // 30 fps animation
        FrameTimer += DeltaSeconds;
        
        if (FrameTimer >= TimePerFrame)
        {
            FrameTimer -= TimePerFrame;
            UKismetRenderingLibrary::ClearRenderTarget2D(
                this,
                RenderTarget,
                FLinearColor::Black  // or Black, or whatever you want
            );
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(
                this,
                RenderTarget,
                AnimatedMID
            );
        }
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
    SceneCapture->SetMobility(EComponentMobility::Static);

    Light->SetRelativeLocation(FVector(Distance, 0.f, 0.f));
    Light->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));
    Light->SetMobility(EComponentMobility::Static);
}

void AItemPreviewActor::CaptureFrameToSpriteSheet()
{
    if (!RenderTarget) return;

    SceneCapture->CaptureScene();

    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();

    // Read this frame's pixels
    TArray<FColor> FramePixels;
    FramePixels.SetNum(FrameSize.X * FrameSize.Y);
    FReadSurfaceDataFlags Flags;
    //Flags.SetLinearToGamma(false);         // CRITICAL FIX
    RTResource->ReadPixels(FramePixels, Flags);

    // Compute grid coordinate
    int32 Row = FramesCaptured / FramesPerAxis;
    int32 Col = FramesCaptured % FramesPerAxis;

    int32 DestX = Col * FrameSize.X;
    int32 DestY = Row * FrameSize.Y;

    for (int32 Y = 0; Y < FrameSize.Y; Y++)
    {
        for (int32 X = 0; X < FrameSize.X; X++)
        {
            int32 SrcIndex = Y * FrameSize.X + X;
            int32 DestIndex = (DestY + Y) * SpriteSheetSize.X + (DestX + X);

            FColor C = FramePixels[SrcIndex];

            // convert per pixel instantly
            FLinearColor Lin = C.ReinterpretAsLinear();
            C = Lin.ToFColorSRGB();

            // invert alpha ONCE here
            C.A = 255 - C.A;

            SpriteSheetPixels[DestIndex] = C;
        }
    }
}

void AItemPreviewActor::StartSpriteSheetCapture()
{
    FString MeshName = MeshComponent->GetStaticMesh()->GetName();

    OutputDirectory = FPaths::ProjectSavedDir() / "ItemPreviews" / MeshName;
    IFileManager::Get().MakeDirectory(*OutputDirectory, true);

    // Final file path
    OutputFilePath = OutputDirectory / "SpriteSheet.png";

    NumFramesToCapture = 64;
    FramesCaptured = 0;

    bIsCapturingSpriteSheet = true;
    bRotate = true;

    // Each frame = the render target size
    FrameSize = FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY);

    FramesPerAxis = 8; // 8x8 grid
    SpriteSheetSize = FIntPoint(FrameSize.X * FramesPerAxis, FrameSize.Y * FramesPerAxis);

    // Allocate buffer for the sprite sheet
    SpriteSheetPixels.SetNum(SpriteSheetSize.X * SpriteSheetSize.Y);
}

void AItemPreviewActor::SaveSpriteSheetToFile()
{
    FCreateTexture2DParameters Params;
    Params.bDeferCompression = true;
    SpriteSheetTexture = UTexture2D::CreateTransient(
        SpriteSheetSize.X,
        SpriteSheetSize.Y,
        PF_B8G8R8A8
    );

    SpriteSheetTexture->SRGB = true;
    SpriteSheetTexture->NeverStream = true;

    // Lock for writing
    void* TextureData = SpriteSheetTexture->GetPlatformData()
        ->Mips[0]
        .BulkData
        .Lock(LOCK_READ_WRITE);

    // Copy our BGRA data
    FMemory::Memcpy(
        TextureData,
        SpriteSheetPixels.GetData(),
        SpriteSheetPixels.Num() * sizeof(FColor)
    );

    // Unlock
    SpriteSheetTexture->GetPlatformData()
        ->Mips[0]
        .BulkData
        .Unlock();
    // Load sprite sheet texture
    //SpriteSheetTexture = FImageUtils::ImportFileAsTexture2D(OutputFilePath);
    SpriteSheetTexture->CompressionSettings = TC_Default;
    SpriteSheetTexture->MipGenSettings = TMGS_NoMipmaps;
    SpriteSheetTexture->NeverStream = true;
    SpriteSheetTexture->UpdateResource();
    if (!SpriteSheetTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to import sprite sheet texture"));
        return;
    }

    // Disable SceneCapture forever
    SceneCapture->TextureTarget = nullptr;
    SceneCapture->DestroyComponent();
    SceneCapture = nullptr;

    // Create MID from your copy material
    AnimatedMID = UMaterialInstanceDynamic::Create(CopyTextureMaterial, this);
    AnimatedMID->SetTextureParameterValue("SheetTexture", SpriteSheetTexture);
    AnimatedMID->SetScalarParameterValue("FramesPerAxis", 8);
    AnimatedMID->SetScalarParameterValue("FrameIndex", 0);

    shouldAnimateSheet = true;
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
