// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/TextScoringSystem.h"

#include "Math/UnrealMathUtility.h"
#include "Engine/DataTable.h"
#include "Async/Async.h"
#include "WrittingReviewStruct.h"

void UTextScoringSystem::Initialize(UDataTable* InFrequencyTable, UDataTable* InTypeTable, UDataTable* InItemListTable)
{
	WordFrequencyScorer = MakeUnique<WordFrequencyScoring>(InFrequencyTable);
	SentenceAnalysis = MakeUnique<SentenceAnalyser>(InTypeTable);
	ItemFinder = MakeUnique<ItemListFinder>(InItemListTable);
}

void UTextScoringSystem::StartScoringWithDelay(const FString& Text, UObject* WorldContext)
{
    TWeakObjectPtr<UTextScoringSystem> WeakThis(this);
    const FString TextCopy = Text;

    Async(EAsyncExecution::ThreadPool, [WeakThis, TextCopy]()
        {
            if (!WeakThis.IsValid() || !WeakThis->SentenceAnalysis)
            {
                return;
            }

            TArray<FPreparedSentence> LocalPreparedSentences;

            // Heavy work off-thread
            WeakThis->BuildPreparedSentences(TextCopy, LocalPreparedSentences);

            // Return to game thread
            AsyncTask(ENamedThreads::GameThread, [WeakThis, Prepared = MoveTemp(LocalPreparedSentences)]() mutable
                {
                    if (!WeakThis.IsValid())
                    {
                        return;
                    }

                    FTSTicker::GetCoreTicker().RemoveTicker(WeakThis->ScoringTimerHandle);

                    WeakThis->PreparedSentences = MoveTemp(Prepared);
                    WeakThis->CurrentSentenceIndex = 0;
                    WeakThis->CurrentScoringIndex = 0;

                    WeakThis->ProcessNextSentence();
                });
        });
}

void UTextScoringSystem::BuildPreparedSentences(const FString& Text, TArray<FPreparedSentence>& OutSentences)
{
    OutSentences.Reset();

    SentenceAnalysis->SplitTextIntoSentences(Text);

    while (SentenceAnalysis->HasNextSentence())
    {
        FPreparedSentence PreparedSentence;

        const auto& SentenceAttributes = SentenceAnalysis->GetCurrentSentenceAttributes();
        const FString NextSentence = SentenceAnalysis->GetNextSentence();

        PreparedSentence.SentenceText = NextSentence;
        PreparedSentence.SentenceTypes = TArray<ESentenceType>(SentenceAttributes.SentenceTypes);
        PreparedSentence.SentenceStructure = SentenceAttributes.SentenceStructure;
        PreparedSentence.SentenceMultiplier = SentenceAnalysis->GetSentenceMultiplier(SentenceAttributes.SentenceStructure);

        // Important:
        // make a PURE version of ParseToScoredTokens that returns tokens instead of mutating UObject state
        PreparedSentence.Tokens = ParseToScoredTokens(NextSentence);

        OutSentences.Add(MoveTemp(PreparedSentence));
    }

    SentenceAnalysis->Reset();
}

TArray<FScoredToken> UTextScoringSystem::ParseToScoredTokens(const FString& Text)
{
    TArray<FScoredToken> Result;
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
                Result.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            // Add a scored token for the wrap marker
            Result.Add({ TEXT("\\w"), TEXT(""), 0, TEXT("wrap"), false });

            // Skip the 'w'
            i++;
        }
        // --- NEWLINE ---
        else if (C == '\n')
        {
            if (!Current.IsEmpty())
            {
                Result.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            Result.Add({ TEXT("\n"), TEXT(""), 0, TEXT("newline"), false });
        }
        // --- WHITESPACE ---
        else if (FChar::IsWhitespace(C))
        {
            if (!Current.IsEmpty())
            {
                Result.Add(ScoreSingleToken(Current));
                Current.Empty();
            }

            Result.Add({ TEXT(" "), TEXT(""), 0, TEXT("space"), false });
        }
        // --- SENTENCE ENDING PUNCTUATION ---
        else if (C == '.' || C == '!' || C == '?')
        {
            if (!Current.IsEmpty())
            {
                Current.AppendChar(C);
                Result.Add(ScoreSingleToken(Current));
                Current.Empty();
            }
            else
            {
                FString Punc(1, &C);
                Result.Add({ Punc, TEXT(""), 0, TEXT("punctuation"), false });
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
        Result.Add(ScoreSingleToken(Current));
    }

    return Result;
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

    //ItemFinder->ProcessThemeForWord(Clean);

	return Token;
}

void UTextScoringSystem::ProcessNextSentence() {
    FTSTicker::GetCoreTicker().RemoveTicker(ScoringTimerHandle);

    if (!PreparedSentences.IsValidIndex(CurrentSentenceIndex))
    {
        PreparedSentences.Reset();
        CurrentSentenceIndex = 0;
        CurrentScoringIndex = 0;
        OnReviewComplete.Broadcast();
        return;
    }

    const FPreparedSentence& Sentence = PreparedSentences[CurrentSentenceIndex];

    CurrentScoringIndex = 0;
    ScoredTokens = Sentence.Tokens;

    OnSentencedProcessed.Broadcast(Sentence.SentenceTypes, Sentence.SentenceStructure);
    OnMultiplicatorAdded.Broadcast(Sentence.SentenceMultiplier, "Sentence structure", "");

    ++CurrentSentenceIndex;

    ScoringTimerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UTextScoringSystem::ProcessNextScoredToken),
        0.2f
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

    if (Token.ShouldBeScored)
    {
        OnScoreAdded.Broadcast(Token.Score, Token.CleanWord, "Frequency");
    }

    for (const auto ClassDiscovered : Token.ItemClass)
    {
        OnItemDiscovered.Broadcast(ClassDiscovered, Token.CleanWord);
    }

    OnWordProcessed.Broadcast(
        Token.OriginalText,
        FLinearColor(static_cast<float>(Token.Score) / 100.f, 0.f, 0.f, 1.f)
    );

    return true;
}
