// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/WordFrequencyScoring.h"

#include "WrittingReviewStruct.h"

#include "Engine/DataTable.h"

WordFrequencyScoring::WordFrequencyScoring(UDataTable* frequency_data_table)
{
	WordFrequencyMap.Empty();
	if (!frequency_data_table) {
		return;
	}

	const auto& RowMap = frequency_data_table->GetRowMap();
	auto It = RowMap.CreateConstIterator();
	const FWordFrequencyEntry* FirstRow = reinterpret_cast<const FWordFrequencyEntry*>(It.Value());

	const FWordFrequencyEntry* LastRow = nullptr;
	for (; It; ++It)
	{
		LastRow = reinterpret_cast<const FWordFrequencyEntry*>(It.Value());
	}

	MaxFrequency = FirstRow ? FirstRow->Count : 1;
	MinFrequency = LastRow ? LastRow->Count : 0;

	for (const FName& RowName : frequency_data_table->GetRowNames())
	{
		if (const FWordFrequencyEntry* Row = frequency_data_table->FindRow<FWordFrequencyEntry>(RowName, TEXT("")))
		{
			WordFrequencyMap.Add(RowName.ToString().ToLower(), Row->Count);
		}
	}

}

float WordFrequencyScoring::GetFrequencyScore(FString const& word) const
{
	const int64* frequency = WordFrequencyMap.Find(word);
	if (!frequency) {
		return 0.f;
	}

	if (MaxFrequency == MinFrequency || *frequency <= 0) {
		return 0.0f;
	}

	float LogFreq = FMath::Loge(static_cast<float>(*frequency));
	float LogMax = FMath::Loge(static_cast<float>(MaxFrequency));
	float LogMin = FMath::Loge(static_cast<float>(MinFrequency));

	float Normalized = (LogMax - LogFreq) / (LogMax - LogMin);
	Normalized = FMath::Clamp(Normalized, 0.0f, 1.0f);

	// Exponential skew
	const float Exponent = 4.0f; // Try 3–6
	float Skewed = FMath::Pow(Normalized, Exponent);

	return FMath::Clamp(Skewed * 100.0f, 1.0f, 100.0f);
}
