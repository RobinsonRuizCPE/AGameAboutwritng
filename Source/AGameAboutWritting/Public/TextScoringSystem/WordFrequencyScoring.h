// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UDataTable;
struct FWordFrequencyEntry;

class AGAMEABOUTWRITTING_API WordFrequencyScoring
{
public:
	WordFrequencyScoring(UDataTable* frequency_data_table);

	float GetFrequencyScore(FString const& word) const;

private:
	TMap<FString, int64> WordFrequencyMap;
	int64 MinFrequency = 0;
	int64 MaxFrequency = 1;
};
