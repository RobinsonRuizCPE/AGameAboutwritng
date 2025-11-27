// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopedSlowTask.h"

#include "TextScoringSystem/WordFrequencyScoring.h"
#include "TextScoringSystem/WordTypeScoring.h"
#include "TextScoringSystem/SentenceAnalyser.h"
#include "TextScoringSystem/ItemListFinder.h"

#include "TextScoringSystem.generated.h"

class UDataTable;
struct FWordTypeEntry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReviewComplete);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSentenceProcessed,
	TArray<ESentenceType>, SentenceTypes,
	ESentenceStructureType, SentenceStructType
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWordProcessed,
	FString, Word,
	FLinearColor, Color
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnScoreAdded,
	int32, ScoreToAdd,
	FString, StringResponsible,
	FString, Reason
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnItemDiscovered,
	UClass*, ItemDiscovered,
	FString, Word
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMultiplicatorAdded,
	int32, MultiplicatorToAdd,
	FString, StringResponsible,
	FString, Reason
);

USTRUCT(BlueprintType)
struct FScoredToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		FString OriginalText;

	UPROPERTY(BlueprintReadOnly)
		FString CleanWord;

	UPROPERTY(BlueprintReadOnly)
		int32 Score = 0;

	UPROPERTY(BlueprintReadOnly)
		FString WordType = "unknown";

	UPROPERTY(BlueprintReadOnly)
		bool ShouldBeScored = true;

	UPROPERTY(BlueprintReadOnly)
		TArray<UClass*> ItemClass;
};

UCLASS(Blueprintable, BlueprintType)
class AGAMEABOUTWRITTING_API UTextScoringSystem : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void Initialize(UDataTable* InFrequencyTable, UDataTable* InTypeTable, UDataTable* InItemListTable);

	UFUNCTION(BlueprintCallable)
		void StartScoringWithDelay(const FString& Text, UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
		TMap<FString, float> GetWeightsFromClass(UClass* class_to_check) { return ItemFinder->GetWordRelatedWeights(class_to_check); }

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnScoreAdded OnScoreAdded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnItemDiscovered OnItemDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnMultiplicatorAdded OnMultiplicatorAdded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnWordProcessed OnWordProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnSentenceProcessed OnSentencedProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnReviewComplete OnReviewComplete;

private:

	bool ProcessNextScoredToken(float DeltaTime);
	void ProcessNextSentence();
	void ParseToScoredTokens(const FString& Text);
	FScoredToken ScoreSingleToken(const FString& Raw);

protected:

	TArray<FString> AllSentences;
	TArray<FScoredToken> ScoredTokensForCurrentSentence;

	int32 CurrentSentenceIndex = 0;
	int32 CurrentTokenIndex = 0;

	UPROPERTY()
		TArray<FScoredToken> ScoredTokens;

	UPROPERTY()
		int32 CurrentScoringIndex = 0;

private:
	TUniquePtr<WordFrequencyScoring> WordFrequencyScorer;
	TUniquePtr<SentenceAnalyser> SentenceAnalysis;
	TUniquePtr<ItemListFinder> ItemFinder;

	FTSTicker::FDelegateHandle ScoringTimerHandle;
	UWorld* WorldRef = nullptr;

};
