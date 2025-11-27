#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "BasePaperItem.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API ABasePaperItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasePaperItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Root scene component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USceneComponent* Scene;

	// Widget component for the UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UWidgetComponent* Widget;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArrowComponent* PageReviewedTransform;
};