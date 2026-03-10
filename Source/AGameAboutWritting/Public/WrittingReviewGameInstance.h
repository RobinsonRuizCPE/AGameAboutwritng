// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TextScoringSystem/TextScoringSystem.h"
#include "PreviewSceneUI/StaticMeshPreviewRenderer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RatingSystem/RatingPaperMaterialParams.h"
#include "Blueprint/UserWidget.h"
#include "LeaderboardService/LeaderboardService.h"
#include "WrittingReviewGameInstance.generated.h"

UCLASS()
class AGAMEABOUTWRITTING_API UWrittingReviewGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	 void CreatePermanentWidget(UUserWidget* widget);

	UFUNCTION(BlueprintCallable)
		void RemovePermanentWidget(UUserWidget* widget);

	UFUNCTION(BlueprintPure)
		UItemPreviewManager* GetPreviewManager() const { return PreviewManager; }

	UFUNCTION(BlueprintCallable)
	UMaterialInstanceDynamic* GetOrCreateMaterialInstance(UMaterialInterface* ParentMaterial, const FRatingPaperMaterialParameters& InParams);


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

	UPROPERTY(BlueprintReadOnly)
	ULeaderboardService* LeaderboardService;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Supabase")
	FString SupabaseProjectUrl = "https://xxonyrjlplsqooykgdtl.supabase.co";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Supabase")
	FString SupabaseAnonKey = "sb_publishable_6oMxN6FE_ZCsYMHpPUpn4w_x7Fsme-5";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PersistentMenuWidgetClass;

protected:
	UPROPERTY()
	TMap<FRatingPaperMaterialParameters, TObjectPtr<UMaterialInstanceDynamic>> CachedMIDs;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PermanentWidgetObject = nullptr;

	TSharedPtr<SWidget> PermanentSlateWidget;
};