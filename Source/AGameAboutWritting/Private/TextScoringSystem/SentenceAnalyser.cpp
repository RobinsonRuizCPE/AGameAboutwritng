// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/SentenceAnalyser.h"


static void* DllHandle = nullptr;
typedef void (*InitDataAssetFunc)(const char*);
typedef void (*AnalyseSentenceFunc)(const char*, uint8_t**, int*, uint8_t*);
typedef void (*FreeResultFunc)(uint8_t*);

InitDataAssetFunc InitDataAsset = nullptr;
AnalyseSentenceFunc AnalyseSentence = nullptr;
FreeResultFunc FreeResult = nullptr;

SentenceAnalyser::SentenceAnalyser(UDataTable* InTypeTable) {
	WordTypeScorer = MakeUnique<WordTypeScoring>(InTypeTable);

	FString DllPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/UDPIPE_AGMAABOUTWRITTING.dll"));
	DllHandle = FPlatformProcess::GetDllHandle(*DllPath);

	if (DllHandle)
	{
		AnalyseSentence = (AnalyseSentenceFunc)FPlatformProcess::GetDllExport(DllHandle, TEXT("UDPDLL_AnalyseSentence"));
		FreeResult = (FreeResultFunc)FPlatformProcess::GetDllExport(DllHandle, TEXT("UDPDLL_FreeBuffer"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DLL."));
	}
}

void SentenceAnalyser::SplitTextIntoSentences(const FString& Text)
{
	FString CurrentSentence;
	bool SentenceJustEnded = false;
	for (int32 i = 0; i < Text.Len(); ++i)
	{
		CurrentSentence.AppendChar(Text[i]);

		if (EndsWithSentenceTerminator(CurrentSentence))
		{
			if (SentenceJustEnded && CurrentSentence.Len() == 1)
			{
				// Append this character to the previous sentence
				if (ParsedSentences.Num() > 0)
				{
					ParsedSentences.Last().Append(CurrentSentence);
				}
				SentenceJustEnded = false;
				CurrentSentence.Empty();
				continue;
			}

			ParsedSentences.Add(CurrentSentence);
			FString Clean = CurrentSentence.Replace(TEXT("\\w"), TEXT(""));
			ParsedSentencesAttributes.Add(CategorizeSentence(Clean));
			CurrentSentence.Empty();
		}
		else
		{
			SentenceJustEnded = false;
		}
	}

	if (!CurrentSentence.IsEmpty())
	{
		ParsedSentences.Add(CurrentSentence);
		ParsedSentencesAttributes.Add(CategorizeSentence(CurrentSentence));
	}

	CurrentSentenceIndex = 0;
}

bool SentenceAnalyser::HasNextSentence() const
{
	return ParsedSentences.IsValidIndex(CurrentSentenceIndex);
}

FString SentenceAnalyser::GetNextSentence()
{
	return HasNextSentence()
		? ParsedSentences[CurrentSentenceIndex++]
		: FString();
}

FSentenceAttributes const& SentenceAnalyser::GetCurrentSentenceAttributes() {
	return ParsedSentencesAttributes[CurrentSentenceIndex];
}

void SentenceAnalyser::Reset()
{
	ParsedSentences.Empty();
	CurrentSentenceIndex = 0;
}

FSentenceAttributes SentenceAnalyser::CategorizeSentence(const FString& Sentence)
{
	if (!AnalyseSentence)
	{
		return FSentenceAttributes{};
	}

	uint8_t* SentenceTypes = nullptr;
	int Length = 0;
	uint8_t StructureType = 0;

	FTCHARToUTF8 Converted(*Sentence);
	const char* SentenceUtf8 = Converted.Get();
	AnalyseSentence(SentenceUtf8, &SentenceTypes, &Length, &StructureType);

	FSentenceAttributes SentenceAttrib;
	for (int i = 0; i < Length; ++i)
	{
		uint8_t TypeValue = SentenceTypes[i];
		SentenceAttrib.SentenceTypes.Add(static_cast<ESentenceType>(TypeValue));
	}

	SentenceAttrib.SentenceStructure = static_cast<ESentenceStructureType>(StructureType);
	FreeResult(SentenceTypes);

	return SentenceAttrib;
}

bool SentenceAnalyser::EndsWithSentenceTerminator(const FString& Accumulated)
{
	for (const FString& End : SentenceTerminators)
	{
		if (Accumulated.EndsWith(End))
		{
			return true;
		}
	}
	return false;
}

void SentenceAnalyser::TokenizeSentence(const FString& Sentence, TArray<FString>& OutTokens) const
{
	OutTokens.Empty();
	FString Current;

	for (int32 i = 0; i < Sentence.Len(); ++i)
	{
		TCHAR C = Sentence[i];

		bool bIsWrapMarker = false;

		// Detect "\w" marker -> two characters: '\' followed by 'w'
		if (C == '\\' && i + 1 < Sentence.Len() && Sentence[i + 1] == 'w')
		{
			bIsWrapMarker = true;
		}

		if (C == '\n' || FChar::IsWhitespace(C) || bIsWrapMarker ||
			C == '.' || C == '!' || C == '?')
		{
			// Push current token if any
			if (!Current.IsEmpty())
			{
				OutTokens.Add(Current);
				Current.Empty();
			}

			// Skip the 'w' in "\w"
			if (bIsWrapMarker)
			{
				i++; // Skip the second char of the marker
			}
		}
		else
		{
			Current.AppendChar(C);
		}
	}

	if (!Current.IsEmpty())
	{
		OutTokens.Add(Current);
	}
}