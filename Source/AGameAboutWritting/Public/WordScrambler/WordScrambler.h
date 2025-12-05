// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../ThemeElements/ThemeHolder.h"
#include "InteractableActor/ActorInteractable.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "WordScrambler.generated.h"

DECLARE_DELEGATE_OneParam(FOnMeshMerged, UStaticMesh*);

/**
 * 
 */
UENUM(BlueprintType)
enum class EWordScramblerState : uint8
{
	Closed                 UMETA(DisplayName = "Closed"),
	Opened                 UMETA(DisplayName = "Opened"),
	Filled				   UMETA(DisplayName = "Filled"),
	Processing             UMETA(DisplayName = "Processing"),
	FinishedProcessing     UMETA(DisplayName = "FinishedProcessing"),
};


UCLASS()
class AGAMEABOUTWRITTING_API AWordScrambler : public AThemeHolder
{
	GENERATED_BODY()
public:
	AWordScrambler();


	UFUNCTION(BlueprintCallable, Category = "Item holder")
		AItemTheme* ScrambleItems(AItemTheme* first_item, AItemTheme* second_item);

private:
	UStaticMesh* MergeMeshes(UStaticMesh* MeshA, UStaticMesh* MeshB, UObject* Outer);
	void MergeMaterials(UStaticMesh* MeshA, UStaticMesh* MeshB, UStaticMesh* out_mesh);
	void MergeItemsThemes(AItemTheme* first_item, AItemTheme* second_item, AItemTheme* out_item);

	void CopySimpleCollision(UStaticMesh* MeshA, const FTransform& TransformA, UStaticMesh* MeshB, const FTransform& TransformB, UStaticMesh* OutMesh);

public:

	/** Static mesh attached to root */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		EWordScramblerState CurrentState = EWordScramblerState::Closed;
};
