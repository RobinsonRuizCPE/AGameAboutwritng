#include "BasePaperItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
ABasePaperItem::ABasePaperItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and set root
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = Scene;

	// Create and attach widget
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	Widget->SetupAttachment(Scene); // You can also attach it to StaticMesh if needed

	// Optional: Setup widget behavior (tweak as needed)
	Widget->SetDrawSize(FVector2D(500, 500));
	Widget->SetWidgetSpace(EWidgetSpace::World);
	Widget->SetTwoSided(true);
	Widget->SetGenerateOverlapEvents(false);

	// Create the arrow component
	PageReviewedTransform = CreateDefaultSubobject<UArrowComponent>(TEXT("EditorArrow"));
	PageReviewedTransform->SetupAttachment(Scene); // Or attach to RootComponent if needed
	PageReviewedTransform->SetRelativeLocation(FVector::ZeroVector);
	PageReviewedTransform->SetArrowColor(FColor::Green); // Optional

	// Enable editing in the editor
	PageReviewedTransform->bIsScreenSizeScaled = true;
	PageReviewedTransform->SetHiddenInGame(true); // Usually only for editor visualization
}

// Called when the game starts or when spawned
void ABasePaperItem::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABasePaperItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}