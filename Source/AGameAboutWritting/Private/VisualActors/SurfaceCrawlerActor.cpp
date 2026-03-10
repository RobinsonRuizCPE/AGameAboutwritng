#include "VisualActors/SurfaceCrawlerActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

ASurfaceCrawlerActor::ASurfaceCrawlerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);

	CollisionSphere->InitSphereRadius(20.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionObjectType(ECC_Pawn);
	CollisionSphere->SetGenerateOverlapEvents(false);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DebugSpline = CreateDefaultSubobject<USplineComponent>(TEXT("DebugSpline"));
	DebugSpline->SetupAttachment(CollisionSphere);
	DebugSpline->SetClosedLoop(false);
	DebugSpline->ClearSplinePoints(false);
}

void ASurfaceCrawlerActor::BeginPlay()
{
	Super::BeginPlay();

	FHitResult Hit;
	bHasSurface = AcquireNearestSurface(Hit);

	const float Radius = CollisionSphere->GetScaledSphereRadius();

	if (bHasSurface)
	{
		SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();

		// Sphere center must be radius + offset away from surface
		const FVector SnappedCenter = Hit.ImpactPoint + SurfaceNormal * (Radius + SurfaceOffset);
		CollisionSphere->SetWorldLocation(SnappedCenter, false, nullptr, ETeleportType::TeleportPhysics);

		// Initial tangent
		TangentDir = ProjectOntoSurfaceTangent(GetActorForwardVector(), SurfaceNormal).GetSafeNormal();
		if (TangentDir.IsNearlyZero())
		{
			TangentDir = MakeAnyTangent(SurfaceNormal);
		}

		ResetSnake();

		if (bDebugSpline)
		{
			DebugSpline->ClearSplinePoints(false);
			AddBreadcrumbPoint(GetActorLocation());
		}
	}

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 12.f, 12, bHasSurface ? FColor::Green : FColor::Red, false, 2.f);
	}
}

void ASurfaceCrawlerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Radius = CollisionSphere->GetScaledSphereRadius();

	// Acquire if lost
	if (!bHasSurface)
	{
		FHitResult Hit;
		bHasSurface = AcquireNearestSurface(Hit);
		if (!bHasSurface) return;

		SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
		const FVector SnappedCenter = Hit.ImpactPoint + SurfaceNormal * (Radius + SurfaceOffset);
		CollisionSphere->SetWorldLocation(SnappedCenter, false, nullptr, ETeleportType::TeleportPhysics);

		TangentDir = ProjectOntoSurfaceTangent(GetActorForwardVector(), SurfaceNormal).GetSafeNormal();
		if (TangentDir.IsNearlyZero())
		{
			TangentDir = MakeAnyTangent(SurfaceNormal);
		}

		ResetSnake();
		return;
	}

	// --- 1) Desired snake direction (continuous) ---
	SnakeTime += DeltaSeconds;

	const float SinAngle = FMath::Sin((SnakeTime + SnakePhase) * 2.f * PI * SnakeFreq) * SnakeAmpDeg;
	const float NoiseAngle = FMath::PerlinNoise1D((SnakeTime + SnakePhase) * 0.7f) * SnakeNoiseAmpDeg;
	const float TotalAngle = SinAngle + NoiseAngle;

	FVector DesiredTangent = TangentDir.RotateAngleAxis(TotalAngle, SurfaceNormal).GetSafeNormal();
	DesiredTangent = ProjectOntoSurfaceTangent(DesiredTangent, SurfaceNormal).GetSafeNormal();

	if (!DesiredTangent.IsNearlyZero())
	{
		TangentDir = FMath::VInterpNormalRotationTo(TangentDir, DesiredTangent, DeltaSeconds, SteeringLerp);
	}

	// --- 2) Wall probe in front: if hit -> switch surface (parallel transport) + small normal push-out + restart snake ---
	{
		const FVector Pos = GetActorLocation();
		const FVector End = Pos + TangentDir * ForwardProbeDistance;

		FCollisionQueryParams Params(SCENE_QUERY_STAT(SurfaceCrawler_WallProbe), false, this);
		FHitResult WallHit;

		const bool bHitWall = GetWorld()->SweepSingleByChannel(
			WallHit,
			Pos,
			End,
			FQuat::Identity,
			SurfaceTraceChannel,
			FCollisionShape::MakeSphere(ForwardProbeRadius),
			Params
		);

		if (bDebugDraw)
		{
			DrawDebugLine(GetWorld(), Pos, End, bHitWall ? FColor::Red : FColor::Cyan, false, 0.f, 0, 1.5f);
			DrawDebugSphere(GetWorld(), End, ForwardProbeRadius, 10, bHitWall ? FColor::Red : FColor::Cyan, false, 0.f);
		}

		if (bHitWall && WallHit.bBlockingHit)
		{
			const FVector OldNormal = SurfaceNormal;
			const FVector NewNormal = WallHit.ImpactNormal.GetSafeNormal();

			// Switch normal immediately (no teleport snap)
			SurfaceNormal = NewNormal;

			// Transport the tangent by the minimal rotation OldNormal -> NewNormal
			TangentDir = TransportTangentToNewNormal(OldNormal, SurfaceNormal, TangentDir);

			// Small push-out ONLY along the new normal (clamped) so we don't jump to top edges
			const FVector CurrentPos = GetActorLocation();
			const FVector DesiredCenter = WallHit.ImpactPoint + SurfaceNormal * (Radius + SurfaceOffset);

			FVector Correction = DesiredCenter - CurrentPos;
			Correction = FVector::DotProduct(Correction, SurfaceNormal) * SurfaceNormal;

			const float Len = Correction.Size();
			if (Len > MaxStickCorrection)
			{
				Correction = (Correction / Len) * MaxStickCorrection;
			}

			FHitResult CorrHit;
			CollisionSphere->MoveComponent(Correction, GetActorQuat(), true, &CorrHit);

			ResetSnake();
		}
	}

	// --- 3) Move along tangent (sweep + slide) ---
	{
		const FVector MoveDelta = TangentDir * (CrawlSpeed * DeltaSeconds);

		FHitResult MoveHit;
		CollisionSphere->MoveComponent(MoveDelta, GetActorQuat(), true, &MoveHit);

		if (MoveHit.bBlockingHit)
		{
			const FVector SlideDelta = FVector::VectorPlaneProject(MoveDelta, MoveHit.ImpactNormal);
			FHitResult SlideHit;
			CollisionSphere->MoveComponent(SlideDelta, GetActorQuat(), true, &SlideHit);
		}
	}

	// --- 4) Stick to current surface (short sweep along -normal, correction ONLY along normal) ---
	//     Also parallel-transport tangent if the surface normal changes meaningfully.
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FCollisionQueryParams Params(SCENE_QUERY_STAT(SurfaceCrawler_Stick), false, this);
			const FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);

			const FVector Pos = GetActorLocation();
			const FVector Up = SurfaceNormal.GetSafeNormal();

			const FVector SweepStart = Pos + Up * StickDistance;
			const FVector SweepEnd = Pos - Up * StickDistance;

			FHitResult StickHit;
			const bool bHit = World->SweepSingleByChannel(
				StickHit, SweepStart, SweepEnd, FQuat::Identity, SurfaceTraceChannel, Shape, Params
			);

			if (bDebugDraw)
			{
				DrawDebugLine(World, SweepStart, SweepEnd, bHit ? FColor::Green : FColor::Silver, false, 0.f, 0, 1.f);
			}

			if (bHit && StickHit.bBlockingHit)
			{
				const FVector OldNormal = SurfaceNormal;
				const FVector TargetNormal = StickHit.ImpactNormal.GetSafeNormal();

				SurfaceNormal = FMath::VInterpNormalRotationTo(SurfaceNormal, TargetNormal, DeltaSeconds, NormalSwitchLerp);

				// Transport tangent to follow the surface change (no "wall/ceiling" special-casing)
				TangentDir = TransportTangentToNewNormal(OldNormal, SurfaceNormal, TangentDir);

				// Desired sphere center at contact + offset
				const FVector DesiredCenter = StickHit.Location + SurfaceNormal * SurfaceOffset;

				// Only correct along normal, so we never cancel forward movement
				FVector Correction = DesiredCenter - Pos;
				Correction = FVector::DotProduct(Correction, SurfaceNormal) * SurfaceNormal;

				const float CorrLen = Correction.Size();
				if (CorrLen > MaxStickCorrection)
				{
					Correction = (Correction / CorrLen) * MaxStickCorrection;
				}

				FHitResult CorrHit;
				CollisionSphere->MoveComponent(Correction, GetActorQuat(), true, &CorrHit);
			}
		}
	}

	// --- 5) Orientation ---
	{
		const FVector Forward = TangentDir.GetSafeNormal();
		const FVector Up = SurfaceNormal.GetSafeNormal();
		const FRotator TargetRot = UKismetMathLibrary::MakeRotFromXZ(Forward, Up);
		SetActorRotation(TargetRot);
	}

	// --- Breadcrumb spline ---
	if (bDebugSpline)
	{
		TimeToNextBreadcrumb -= DeltaSeconds;
		if (TimeToNextBreadcrumb <= 0.f)
		{
			AddBreadcrumbPoint(GetActorLocation());
			TrimSplinePointsIfNeeded();
			TimeToNextBreadcrumb = BreadcrumbInterval;
		}
	}

	// Extra debug vectors
	if (bDebugDraw)
	{
		const FVector P = GetActorLocation();
		DrawDebugDirectionalArrow(GetWorld(), P, P + TangentDir * 70.f, 18.f, FColor::Yellow, false, 0.f, 0, 2.f);
		DrawDebugDirectionalArrow(GetWorld(), P, P + SurfaceNormal * 70.f, 18.f, FColor::Green, false, 0.f, 0, 2.f);
	}
}
bool ASurfaceCrawlerActor::AcquireNearestSurface(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	const FVector Origin = GetActorLocation();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SurfaceCrawler_Acquire), false, this);

	// 6-direction line traces to find a nearby surface
	const FVector Dirs[6] = {
		FVector::DownVector,
		FVector::UpVector,
		FVector::ForwardVector,
		-FVector::ForwardVector,
		FVector::RightVector,
		-FVector::RightVector
	};

	bool bFound = false;
	float BestDistSq = TNumericLimits<float>::Max();
	FHitResult Best;

	for (const FVector& Dir : Dirs)
	{
		FHitResult Hit;
		const FVector End = Origin + Dir * AcquireProbeLength;

		if (World->LineTraceSingleByChannel(Hit, Origin, End, SurfaceTraceChannel, Params) && Hit.bBlockingHit)
		{
			const float D2 = FVector::DistSquared(Origin, Hit.ImpactPoint);
			if (D2 < BestDistSq)
			{
				BestDistSq = D2;
				Best = Hit;
				bFound = true;
			}
		}
	}

	if (!bFound) return false;

	OutHit = Best;
	return true;
}

void ASurfaceCrawlerActor::ResetSnake()
{
	SnakeTime = 0.f;
	SnakePhase = FMath::FRandRange(0.f, 2.f * PI);
}

FVector ASurfaceCrawlerActor::ProjectOntoSurfaceTangent(const FVector& Vec, const FVector& Normal) const
{
	return Vec - FVector::DotProduct(Vec, Normal) * Normal;
}

FVector ASurfaceCrawlerActor::MakeAnyTangent(const FVector& Normal) const
{
	const FVector N = Normal.GetSafeNormal();
	const FVector Helper = (FMath::Abs(N.Z) < 0.9f) ? FVector::UpVector : FVector::RightVector;
	FVector T = FVector::CrossProduct(Helper, N).GetSafeNormal();
	if (T.IsNearlyZero())
	{
		T = FVector::CrossProduct(FVector::ForwardVector, N).GetSafeNormal();
	}
	return T;
}

void ASurfaceCrawlerActor::AddBreadcrumbPoint(const FVector& WorldPoint)
{
	if (!DebugSpline) return;

	const int32 Num = DebugSpline->GetNumberOfSplinePoints();
	DebugSpline->AddSplinePoint(WorldPoint, ESplineCoordinateSpace::World, false);
	DebugSpline->SetSplinePointType(Num, ESplinePointType::Curve, false);
	DebugSpline->UpdateSpline();
}

void ASurfaceCrawlerActor::TrimSplinePointsIfNeeded()
{
	if (!DebugSpline) return;

	const int32 Num = DebugSpline->GetNumberOfSplinePoints();
	if (Num <= MaxBreadcrumbs) return;

	const int32 ToRemove = Num - MaxBreadcrumbs;
	for (int32 i = 0; i < ToRemove; ++i)
	{
		DebugSpline->RemoveSplinePoint(0, false);
	}
	DebugSpline->UpdateSpline();
}

FVector ASurfaceCrawlerActor::TransportTangentToNewNormal(
	const FVector& OldNormal,
	const FVector& NewNormal,
	const FVector& OldTangent) const
{
	const FVector N0 = OldNormal.GetSafeNormal();
	const FVector N1 = NewNormal.GetSafeNormal();

	// Minimal rotation that maps N0 -> N1
	const FQuat Q = FQuat::FindBetweenNormals(N0, N1);

	// Rotate the tangent with the same rotation
	FVector T = Q.RotateVector(OldTangent);

	// Ensure it's exactly tangent to the new surface
	T = ProjectOntoSurfaceTangent(T, N1).GetSafeNormal();

	// Fallback if degenerate (can happen if OldTangent ~ parallel to N0)
	if (T.IsNearlyZero())
	{
		T = MakeAnyTangent(N1);
	}

	return T;
}