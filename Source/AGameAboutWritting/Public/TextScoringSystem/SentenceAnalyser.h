// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Utility/ESentenceStructAndTypes.h"
#include "TextScoringSystem/WordTypeScoring.h"

class UDataTable;

struct FSentenceAttributes
{
	TArray<ESentenceType> SentenceTypes;
	ESentenceStructureType SentenceStructure;
};

/**
 * 
 */
const TSet<FString> SentenceTerminators = {
	TEXT("."), TEXT("!"), TEXT("?"),
	TEXT("..."), TEXT("…"),
	TEXT(".'"), TEXT("!'"), TEXT("?'"),
	TEXT(".\""), TEXT("!\""), TEXT("?\""),
	TEXT(".”"), TEXT("!’"), TEXT("?”"),
	TEXT("...\""), TEXT("…\""),
	TEXT(":"), TEXT(";")
};

static const TMap<ESentenceStructureType, int32> SentenceStructureMultiplier = {
	{ ESentenceStructureType::Unknown			,0 },
	{ ESentenceStructureType::Simple         	,1 },
	{ ESentenceStructureType::Compound       	,2 },
	{ ESentenceStructureType::Complex        	,2 },
	{ ESentenceStructureType::CompoundComplex	,4 }
};

class AGAMEABOUTWRITTING_API SentenceAnalyser
{
public:
	SentenceAnalyser(UDataTable* InTypeTable);

	void SplitTextIntoSentences(const FString& Text);

	bool HasNextSentence() const;
	FString GetNextSentence();
	FSentenceAttributes const& GetCurrentSentenceAttributes();

	int32 GetSentenceMultiplier(ESentenceStructureType const& structure) const {
		auto const mult = SentenceStructureMultiplier.Find(structure);
		return mult ? *mult : 0;
	};

	void Reset();

private:

	bool EndsWithSentenceTerminator(const FString& Accumulated);

	void TokenizeSentence(const FString& Sentence, TArray<FString>& OutTokens) const;

	FSentenceAttributes CategorizeSentence(FString const& sentence);

private:
	TArray<FString> ParsedSentences;
	TArray<FSentenceAttributes> ParsedSentencesAttributes;
	int32 CurrentSentenceIndex = 0;

	TUniquePtr<WordTypeScoring> WordTypeScorer;
};
