// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "MultiViewSpriteBaker.h"
#include "StaticMeshPreviewRenderer.generated.h"

/**
 * 
 */

UCLASS()
class AGAMEABOUTWRITTING_API AItemPreviewActor : public AActor
{
    GENERATED_BODY()

public:
    AItemPreviewActor();
    void InitializePreview(UStaticMesh* Mesh, int32 Width = 512, int32 Height = 512);
    void SetMesh(UStaticMesh* Mesh);
    UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }
    void SetRotationEnabled(bool bEnabled) { bRotate = bEnabled; }
    void CenterMeshInCapture();

    void CaptureFrameToSpriteSheet();
    void StartSpriteSheetCapture();

    void SaveSpriteSheetToFile();

protected:
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditDefaultsOnly)
        UMaterialInterface* CopyTextureMaterial;

private:
    UPROPERTY()
        USceneComponent* Root;

    UPROPERTY()
        UStaticMeshComponent* MeshComponent;

    UPROPERTY()
        UTexture2D* SpriteSheetTexture;

    UPROPERTY()
        UMaterialInstanceDynamic* AnimatedMID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview", meta = (AllowPrivateAccess = "true"))
        USpotLightComponent* Light;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview", meta = (AllowPrivateAccess = "true"))
        USceneCaptureComponent2D* SceneCapture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview", meta = (AllowPrivateAccess = "true"))
        UTextureRenderTarget2D* RenderTarget;

    bool bRotate = true;
    float RotationSpeed = 25.f;
    float timer = 0.f;

    // Number of frames per full rotation
    int32 NumFramesToCapture = 64;

    // Where to save the frames
    FString OutputDirectory;
    FString OutputFilePath;

    // Internal counters
    int32 CurrentFrame = 0;
    bool bIsCapturing = false;

    TArray<FColor> SpriteSheetPixels;
    int32 FramesCaptured = 0;
    int32 FramesPerAxis = 8;  // because sqrt(64) = 8
    bool bIsCapturingSpriteSheet = false;
    FIntPoint FrameSize;
    FIntPoint SpriteSheetSize;

    float FrameTimer;
    int CurrentAnimFrame = 0;
    bool shouldAnimateSheet = false;
};


UCLASS()
class AGAMEABOUTWRITTING_API UItemPreviewManager : public UObject
{
    GENERATED_BODY()

public:
    static UItemPreviewManager* Get(UObject* WorldContext);

    void InitPreviewWorld(UGameInstance* InGameInstance);

    UFUNCTION(BlueprintCallable, Category = "Preview")
        UTextureRenderTarget2D* GetOrCreateRenderTargetFromMesh(UStaticMesh* Mesh, int32 Width = 512, int32 Height = 512);

private:

    UPROPERTY()
        TMap<UStaticMesh*, AItemPreviewActor*> PreviewActorsMap;
};



