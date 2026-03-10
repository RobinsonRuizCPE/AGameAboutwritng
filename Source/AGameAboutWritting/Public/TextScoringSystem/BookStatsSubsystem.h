// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BookStatsSubsystem.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct AGAMEABOUTWRITTING_API FPageStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Page Stats")
	FText PageText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Page Stats")
	int64 PageScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Page Stats")
	TMap<ESentenceType, int32> SentenceTypeCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Page Stats")
	TMap<ESentenceStructureType, int32> SentenceStructureCounts;
};

UCLASS(BlueprintType)
class AGAMEABOUTWRITTING_API UBookStatsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Book Stats")
	void ResetStats();

	UFUNCTION(BlueprintCallable, Category = "Book Stats")
	void SetBookTitle(const FText& InBookTitle);

	UFUNCTION(BlueprintCallable, Category = "Book Stats")
	void SetBookAuthor(const FText& InAuthorName);

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	FText GetBookTitle() const { return BookTitle; }

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	FText GetAuthorName() const { return AuthorName; }

	UFUNCTION(BlueprintCallable, Category = "Book Stats")
	int32 AddPageStats(const FPageStats& PageStats);

	UFUNCTION(BlueprintCallable, Category = "Book Stats")
	bool UpdatePageStats(int32 PageIndex, const FPageStats& PageStats);

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	bool GetPageStats(int32 PageIndex, FPageStats& OutPageStats) const;

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	const TArray<FPageStats>& GetAllPageStats() const { return Pages; }

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	int32 GetPageCount() const { return Pages.Num(); }

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	float GetAverageScorePerPage() const;

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	TMap<ESentenceType, int32> GetTotalSentenceTypeCounts() const;

	UFUNCTION(BlueprintPure, Category = "Book Stats")
	TMap<ESentenceStructureType, int32> GetTotalSentenceStructureCounts() const;

private:
	UPROPERTY()
	TArray<FPageStats> Pages;

	UPROPERTY()
	FText BookTitle;

	UPROPERTY()
	FText AuthorName;
};