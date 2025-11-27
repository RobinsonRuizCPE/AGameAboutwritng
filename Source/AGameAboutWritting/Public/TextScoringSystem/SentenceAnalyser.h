// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TextScoringSystem/WordTypeScoring.h"

class UDataTable;

UENUM(BlueprintType)
enum class ESentenceType : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	Dialog			UMETA(DisplayName = "Dialog"),
	Description		UMETA(DisplayName = "Description"),
	Question		UMETA(DisplayName = "Question"),
	Command			UMETA(DisplayName = "Command"),
	Exposition		UMETA(DisplayName = "Exposition"),
	InternalThought	UMETA(DisplayName = "InternalThought"),
	Narration		UMETA(DisplayName = "Narration"),
	Poetic			UMETA(DisplayName = "Poetic"),
	Statistical		UMETA(DisplayName = "Statistical"),
	Interjection	UMETA(DisplayName = "Interjection"),
};

UENUM(BlueprintType)
enum class ESentenceStructureType : uint8
{
	Unknown             UMETA(DisplayName = "Unknown"),
	Simple              UMETA(DisplayName = "Simple"),
	Compound            UMETA(DisplayName = "Compound"),
	Complex             UMETA(DisplayName = "Complex"),
	CompoundComplex     UMETA(DisplayName = "Compound-Complex"),
};



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
