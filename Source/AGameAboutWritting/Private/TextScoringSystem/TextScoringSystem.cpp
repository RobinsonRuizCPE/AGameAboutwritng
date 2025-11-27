// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/TextScoringSystem.h"

#include "Math/UnrealMathUtility.h"
#include "Engine/DataTable.h"
#include "WrittingReviewStruct.h"

void UTextScoringSystem::Initialize(UDataTable* InFrequencyTable, UDataTable* InTypeTable, UDataTable* InItemListTable)
{
	WordFrequencyScorer = MakeUnique<WordFrequencyScoring>(InFrequencyTable);
	SentenceAnalysis = MakeUnique<SentenceAnalyser>(InTypeTable);
	ItemFinder = MakeUnique<ItemListFinder>(InItemListTable);
}

void UTextScoringSystem::StartScoringWithDelay(const FString& Text, UObject* WorldContext)
{
	if (!WorldContext) return;
	WorldRef = WorldContext->GetWorld();

    SentenceAnalysis->SplitTextIntoSentences(Text);
	ProcessNextSentence();
}

void UTextScoringSystem::ParseToScoredTokens(const FString& Text)
{
    ScoredTokens.Empty();

    FString Current;

    for (int32 i = 0; i < Text.Len(); ++i)
    {
        TCHAR C = Text[i];

        bool bIsWrapMarker = false;

        // Detect literal sequence "\w"
        if (C == '\\' && i + 1 < Text.Len() && Text[i + 1] == 'w')
        {
            bIsWrapMarker = true;
        }

        // --- WRAP MARKER (\w) ---
        if (bIsWrapMarker)
        {
            // Finish current word
            if (!Current.IsEmpty())
            {
                ScoredTokens.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            // Add a scored token for the wrap marker
            ScoredTokens.Add({ TEXT("\\w"), TEXT(""), 0, TEXT("wrap"), false });

            // Skip the 'w'
            i++;
        }
        // --- NEWLINE ---
        else if (C == '\n')
        {
            if (!Current.IsEmpty())
            {
                ScoredTokens.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            ScoredTokens.Add({ TEXT("\n"), TEXT(""), 0, TEXT("newline"), false });
        }
        // --- WHITESPACE ---
        else if (FChar::IsWhitespace(C))
        {
            if (!Current.IsEmpty())
            {
                ScoredTokens.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            ScoredTokens.Add({ TEXT(" "), TEXT(""), 0, TEXT("space"), false });
        }
        // --- SENTENCE ENDING PUNCTUATION ---
        else if (C == '.' || C == '!' || C == '?')
        {
            if (!Current.IsEmpty())
            {
                Current.AppendChar(C);
                ScoredTokens.Add(ScoreSingleToken(Current));
                Current.Empty();
            }
            else
            {
                FString Punc(1, &C);
                ScoredTokens.Add({ Punc, TEXT(""), 0, TEXT("punctuation"), false });
            }
        }
        // --- NORMAL CHAR ---
        else
        {
            Current.AppendChar(C);
        }
    }

    if (!Current.IsEmpty())
    {
        ScoredTokens.Add(ScoreSingleToken(Current));
    }
}

FScoredToken UTextScoringSystem::ScoreSingleToken(const FString& Raw)
{
	FScoredToken Token;
	Token.OriginalText = Raw;

	FString Clean = Raw.ToLower().TrimStartAndEnd();
	Clean.RemoveSpacesInline();
	Clean.RemoveFromEnd(TEXT("."));
	Clean.RemoveFromEnd(TEXT(","));
	Clean.RemoveFromEnd(TEXT(";"));
	Clean.RemoveFromEnd(TEXT("!"));
	Clean.RemoveFromEnd(TEXT("?"));

	Token.CleanWord = Clean;
	Token.Score = WordFrequencyScorer->GetFrequencyScore(Clean);
	Token.ItemClass = ItemFinder->GetCorrespondingItemClasses(Clean);

	return Token;
}

void UTextScoringSystem::ProcessNextSentence() {
    FTSTicker::GetCoreTicker().RemoveTicker(ScoringTimerHandle);

	if (!SentenceAnalysis->HasNextSentence()) {
		SentenceAnalysis->Reset();
		OnReviewComplete.Broadcast();
		return;
	}

	CurrentScoringIndex = 0;
	auto const& sentence_attributes = SentenceAnalysis->GetCurrentSentenceAttributes();
	ParseToScoredTokens(SentenceAnalysis->GetNextSentence());
	OnSentencedProcessed.Broadcast(TArray<ESentenceType>(sentence_attributes.SentenceTypes), sentence_attributes.SentenceStructure);
    OnMultiplicatorAdded.Broadcast(SentenceAnalysis->GetSentenceMultiplier(sentence_attributes.SentenceStructure), "Sentence structure", "");
    ScoringTimerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UTextScoringSystem::ProcessNextScoredToken),
        0.2f   // Delay between calls in seconds
    );
}


bool UTextScoringSystem::ProcessNextScoredToken(float DeltaTime)
{
	if (!ScoredTokens.IsValidIndex(CurrentScoringIndex))
	{
		ProcessNextSentence();
		return true;
	}

	const FScoredToken& Token = ScoredTokens[CurrentScoringIndex++];
	
	if (Token.ShouldBeScored) {
		OnScoreAdded.Broadcast(Token.Score, Token.CleanWord, "Frequency");
	}

    for(auto const class_discovered : Token.ItemClass){
		OnItemDiscovered.Broadcast(class_discovered, Token.CleanWord);
	}

	OnWordProcessed.Broadcast(Token.OriginalText, FLinearColor{ static_cast<float>(Token.Score) / 100.f, 0.f,0.f,1.f });
    return true;
}
