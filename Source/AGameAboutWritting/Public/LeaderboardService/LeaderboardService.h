#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"

#include "LeaderboardService.generated.h"

USTRUCT(BlueprintType)
struct AGAMEABOUTWRITTING_API FLeaderboardSubmission
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int64 Id = -1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Rank = -1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString AuthorName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int64 Score = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString BookTitle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString PreviewText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString FullText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 AveragePerPage = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 PageCount = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString CreatedAt;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubmitFinished, bool, bSuccess, const FString&, ResponseContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFetchFinished, bool, bSuccess, const FString&, ResponseContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRankFetchFinished, bool, bSuccess, int32, Rank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPredictedSubmissionFetched, bool, bSuccess, const FLeaderboardSubmission&, Submission);


UCLASS(BlueprintType, Blueprintable)
class AGAMEABOUTWRITTING_API ULeaderboardService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(const FString& InProjectUrl, const FString& InAnonKey);

    UFUNCTION(BlueprintCallable)
    void SubmitSubmission(const FLeaderboardSubmission& Submission);

    UFUNCTION(BlueprintCallable)
    void FetchTopSubmissions(int32 Limit = 20);

    UFUNCTION(BlueprintCallable)
    void FetchSubmissionById(int64 SubmissionId);

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void SubmitBookStats(UBookStatsSubsystem* BookStatsSubsystem);

    UFUNCTION(BlueprintPure, Category = "Leaderboard")
    bool BuildSubmissionFromBookStats(UBookStatsSubsystem* BookStatsSubsystem, FLeaderboardSubmission& OutSubmission) const;

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    bool ParseSubmissionArray(const FString& JsonString, TArray<FLeaderboardSubmission>& OutSubmissions) const;

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    bool ParseSingleSubmission(const FString& JsonString, FLeaderboardSubmission& OutSubmission) const;

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void FetchSubmissionRank(int64 SubmissionId);

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void FetchPredictedSubmission(UBookStatsSubsystem* BookStatsSubsystem);

    UPROPERTY(BlueprintAssignable)
    FOnRankFetchFinished OnRankFetchFinished;

    UPROPERTY(BlueprintAssignable)
    FOnSubmitFinished OnSubmitFinished;

    UPROPERTY(BlueprintAssignable)
    FOnFetchFinished OnFetchFinished;

    UPROPERTY(BlueprintAssignable)
    FOnPredictedSubmissionFetched OnPredictedSubmissionFetched;

private:
    FString ProjectUrl;
    FString AnonKey;

    FString GetRestBaseUrl() const;
    void ApplyCommonHeaders(TSharedRef<class IHttpRequest, ESPMode::ThreadSafe> Request) const;

    FString BuildFullTextFromBookStats(const UBookStatsSubsystem* BookStatsSubsystem) const;
    int32 CountWords(const FString& Text) const;

    void HandleFetchRankResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void HandleSubmitResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void HandleFetchResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    bool ParseSubmissionObject(const TSharedPtr<FJsonObject>& JsonObject, FLeaderboardSubmission& OutSubmission) const;

    bool bHasPendingPredictedSubmission = false;
    FLeaderboardSubmission PendingPredictedSubmission;
};