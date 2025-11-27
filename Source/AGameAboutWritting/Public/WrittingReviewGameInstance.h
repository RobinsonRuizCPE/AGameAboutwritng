// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TextScoringSystem/TextScoringSystem.h"
#include "PreviewSceneUI/StaticMeshPreviewRenderer.h"
#include "WrittingReviewGameInstance.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API UWrittingReviewGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure)
		UItemPreviewManager* GetPreviewManager() const { return PreviewManager; }

	UPROPERTY(BlueprintReadOnly)
		UTextScoringSystem* TextScoringSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
		UDataTable* WordFrequencyTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
		UDataTable* WordTypeTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring")
		UDataTable* DTItemList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Review")
		UItemPreviewManager* PreviewManager;
};