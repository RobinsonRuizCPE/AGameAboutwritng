#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "SurfaceCrawlerActor.generated.h"

class UStaticMeshComponent;
class USplineComponent;

UCLASS()
class AGAMEABOUTWRITTING_API ASurfaceCrawlerActor : public AActor
{
	GENERATED_BODY()

public:
	ASurfaceCrawlerActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- Components ---
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USplineComponent* DebugSpline;

	// --- Runtime state ---
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	bool bHasSurface = false;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	FVector TangentDir = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float SnakeTime = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	float SnakePhase = 0.f;

	// Breadcrumb timer
	float TimeToNextBreadcrumb = 0.f;

	// --- Config: Surface acquisition ---
	UPROPERTY(EditAnywhere, Category = "Surface")
	float AcquireProbeLength = 800.f;

	UPROPERTY(EditAnywhere, Category = "Surface")
	TEnumAsByte<ECollisionChannel> SurfaceTraceChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, Category = "Surface")
	float SurfaceOffset = 6.f; // extra distance from surface along normal (beyond sphere radius)

	// --- Config: Crawl movement ---
	UPROPERTY(EditAnywhere, Category = "Crawl")
	float CrawlSpeed = 160.f; // uu/sec

	UPROPERTY(EditAnywhere, Category = "Crawl")
	float SteeringLerp = 6.f; // how quickly tangent follows desired snake direction

	// --- Config: Snake pattern ---
	UPROPERTY(EditAnywhere, Category = "Crawl|Snake")
	float SnakeFreq = 0.6f;       // cycles/sec

	UPROPERTY(EditAnywhere, Category = "Crawl|Snake")
	float SnakeAmpDeg = 25.f;     // degrees

	UPROPERTY(EditAnywhere, Category = "Crawl|Snake")
	float SnakeNoiseAmpDeg = 10.f; // degrees

	// --- Config: Wall switching ---
	UPROPERTY(EditAnywhere, Category = "WallSwitch")
	float ForwardProbeDistance = 90.f;

	UPROPERTY(EditAnywhere, Category = "WallSwitch")
	float ForwardProbeRadius = 18.f;

	UPROPERTY(EditAnywhere, Category = "WallSwitch")
	float NormalSwitchLerp = 14.f; // how fast we align to new wall normal

	// --- Config: Stick to current surface ---
	UPROPERTY(EditAnywhere, Category = "Stick")
	float StickDistance = 60.f;        // short sweep distance along -normal

	UPROPERTY(EditAnywhere, Category = "Stick")
	float MaxStickCorrection = 10.f;   // max correction per tick along normal

	// --- Debug ---
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugSpline = false;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float BreadcrumbInterval = 0.08f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	int32 MaxBreadcrumbs = 120;

	// --- Helpers ---
	bool AcquireNearestSurface(FHitResult& OutHit) const;

	void ResetSnake();

	FVector ProjectOntoSurfaceTangent(const FVector& Vec, const FVector& Normal) const;
	FVector MakeAnyTangent(const FVector& Normal) const;

	void AddBreadcrumbPoint(const FVector& WorldPoint);
	void TrimSplinePointsIfNeeded();

	FVector TransportTangentToNewNormal(const FVector& OldNormal, const FVector& NewNormal, const FVector& OldTangent) const;
};