// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct RelatedResult {
	const char* label;
	float weight;
};

/**
 * 
 */
class AGAMEABOUTWRITTING_API ItemListFinder
{
public:
	ItemListFinder(UDataTable* item_list_data_table);
	~ItemListFinder();

	TArray<UClass*> GetCorrespondingItemClasses(FString const& word);

	TMap<FString, float> GetWordRelatedWeights(UClass* class_to_search);

	TSet<FString> GetBaseThemeFromClass(UClass* class_to_search);

	void ProcessThemeForWord(FString const& word);
	void SortThemes();

private:
	TMap<FString, float> GetRelatedThemeForWord(FString const& word);

private:
	TMap< UClass*, TArray<FString>> ItemToGeneralThemeMap;
	TMultiMap<FString, UClass*> WordToItemClassMap;
	TMap<FString, TMap<FString, float>> WordRelatedWeights;

	TArray<TPair<FString, float>> SortedThemes;

	TMap<FString, float> CurrentReviewThemes;
};
