// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/BookStatsSubsystem.h"
#include "WrittingReviewGameInstance.h"

void UBookStatsSubsystem::ResetStats()
{
	Pages.Reset();
	BookTitle = FText::GetEmpty();
	AuthorName = FText::GetEmpty();
}

void UBookStatsSubsystem::SetBookTitle(const FText& InBookTitle) {
	BookTitle = InBookTitle;
}

void UBookStatsSubsystem::SetBookAuthor(const FText& InAuthorName) {
	AuthorName = InAuthorName;
}

int32 UBookStatsSubsystem::AddPageStats(const FPageStats& PageStats)
{
	return Pages.Add(PageStats);
}

bool UBookStatsSubsystem::UpdatePageStats(int32 PageIndex, const FPageStats& PageStats)
{
	if (!Pages.IsValidIndex(PageIndex))
	{
		return false;
	}

	Pages[PageIndex] = PageStats;
	return true;
}

bool UBookStatsSubsystem::GetPageStats(int32 PageIndex, FPageStats& OutPageStats) const
{
	if (!Pages.IsValidIndex(PageIndex))
	{
		return false;
	}

	OutPageStats = Pages[PageIndex];
	return true;
}

int32 UBookStatsSubsystem::GetTotalScore() const
{
	int32 TotalScore = 0;

	for (const FPageStats& Page : Pages)
	{
		TotalScore += Page.PageScore;
	}

	return TotalScore;
}

float UBookStatsSubsystem::GetAverageScorePerPage() const
{
	if (Pages.Num() == 0)
	{
		return 0.f;
	}

	return static_cast<float>(GetTotalScore()) / static_cast<float>(Pages.Num());
}

TMap<ESentenceType, int32> UBookStatsSubsystem::GetTotalSentenceTypeCounts() const
{
	TMap<ESentenceType, int32> Result;

	for (const FPageStats& Page : Pages)
	{
		for (const TPair<ESentenceType, int32>& Pair : Page.SentenceTypeCounts)
		{
			Result.FindOrAdd(Pair.Key) += Pair.Value;
		}
	}

	return Result;
}

TMap<ESentenceStructureType, int32> UBookStatsSubsystem::GetTotalSentenceStructureCounts() const
{
	TMap<ESentenceStructureType, int32> Result;

	for (const FPageStats& Page : Pages)
	{
		for (const TPair<ESentenceStructureType, int32>& Pair : Page.SentenceStructureCounts)
		{
			Result.FindOrAdd(Pair.Key) += Pair.Value;
		}
	}

	return Result;
}