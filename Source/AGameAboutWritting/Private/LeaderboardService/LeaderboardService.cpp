#include "LeaderboardService/LeaderboardService.h"
#include "TextScoringSystem/BookStatsSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"

void ULeaderboardService::Initialize(const FString& InProjectUrl, const FString& InAnonKey)
{
	ProjectUrl = InProjectUrl;
	AnonKey = InAnonKey;
}

FString ULeaderboardService::GetRestBaseUrl() const
{
	FString CleanUrl = ProjectUrl;
	CleanUrl.RemoveFromEnd(TEXT("/"));
	return CleanUrl + TEXT("/rest/v1");
}

void ULeaderboardService::ApplyCommonHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request) const
{
	Request->SetHeader(TEXT("apikey"), AnonKey);
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AnonKey));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
}

void ULeaderboardService::SubmitSubmission(const FLeaderboardSubmission& Submission)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(GetRestBaseUrl() + TEXT("/submissions"));
	Request->SetVerb(TEXT("POST"));
	ApplyCommonHeaders(Request);
	Request->SetHeader(TEXT("Prefer"), TEXT("return=representation"));

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("username"), Submission.AuthorName);
	JsonObject->SetNumberField(TEXT("score"), static_cast<double>(Submission.Score));
	JsonObject->SetStringField(TEXT("title"), Submission.BookTitle);
	JsonObject->SetStringField(TEXT("preview_text"), Submission.PreviewText);
	JsonObject->SetStringField(TEXT("full_text"), Submission.FullText);
	JsonObject->SetNumberField(TEXT("avg_per_page"), Submission.AveragePerPage);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULeaderboardService::HandleSubmitResponse);
	Request->ProcessRequest();
}

void ULeaderboardService::FetchTopSubmissions(int32 Limit)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	const FString Url = FString::Printf(
		TEXT("%s/submissions?select=id,username,score,title,preview_text,avg_per_page,created_at&order=score.desc,created_at.asc&limit=%d"),
		*GetRestBaseUrl(),
		Limit
	);

	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	ApplyCommonHeaders(Request);

	Request->OnProcessRequestComplete().BindUObject(this, &ULeaderboardService::HandleFetchResponse);
	Request->ProcessRequest();
}

void ULeaderboardService::FetchSubmissionById(int64 SubmissionId)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	const FString Url = FString::Printf(
		TEXT("%s/submissions?select=*&id=eq.%lld"),
		*GetRestBaseUrl(),
		SubmissionId
	);

	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	ApplyCommonHeaders(Request);

	Request->OnProcessRequestComplete().BindUObject(this, &ULeaderboardService::HandleFetchResponse);
	Request->ProcessRequest();
}

void ULeaderboardService::FetchSubmissionRank(int64 SubmissionId)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(GetRestBaseUrl() + TEXT("/rpc/get_submission_rank"));
	Request->SetVerb(TEXT("POST"));
	ApplyCommonHeaders(Request);

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("input_submission_id"), static_cast<double>(SubmissionId));

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULeaderboardService::HandleFetchRankResponse);
	Request->ProcessRequest();
}

void ULeaderboardService::FetchPredictedSubmission(UBookStatsSubsystem* BookStatsSubsystem)
{
	FLeaderboardSubmission Submission;
	if (!BuildSubmissionFromBookStats(BookStatsSubsystem, Submission))
	{
		OnPredictedSubmissionFetched.Broadcast(false, FLeaderboardSubmission());
		return;
	}

	PendingPredictedSubmission = Submission;
	bHasPendingPredictedSubmission = true;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(GetRestBaseUrl() + TEXT("/rpc/get_score_rank"));
	Request->SetVerb(TEXT("POST"));
	ApplyCommonHeaders(Request);

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("input_score"), static_cast<double>(Submission.Score));

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonBody);
	Request->OnProcessRequestComplete().BindUObject(this, &ULeaderboardService::HandleFetchRankResponse);
	Request->ProcessRequest();
}

void ULeaderboardService::HandleSubmitResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const bool bOk = bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());
	const FString Content = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response");
	OnSubmitFinished.Broadcast(bOk, Content);
}

void ULeaderboardService::HandleFetchResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const bool bOk = bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());
	const FString Content = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response");
	OnFetchFinished.Broadcast(bOk, Content);
}

void ULeaderboardService::HandleFetchRankResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const bool bOk = bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());

	if (!bOk || !Response.IsValid())
	{
		OnRankFetchFinished.Broadcast(false, -1);

		if (bHasPendingPredictedSubmission)
		{
			bHasPendingPredictedSubmission = false;
			OnPredictedSubmissionFetched.Broadcast(false, FLeaderboardSubmission());
		}
		return;
	}

	const FString Content = Response->GetContentAsString();
	int32 Rank = -1;

	if (Content.IsNumeric())
	{
		Rank = FCString::Atoi(*Content);
		OnRankFetchFinished.Broadcast(true, Rank);

		if (bHasPendingPredictedSubmission)
		{
			PendingPredictedSubmission.Rank = Rank;
			bHasPendingPredictedSubmission = false;
			OnPredictedSubmissionFetched.Broadcast(true, PendingPredictedSubmission);
		}
		return;
	}

	TSharedPtr<FJsonValue> JsonValue;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);

	if (FJsonSerializer::Deserialize(Reader, JsonValue) && JsonValue.IsValid() && JsonValue->Type == EJson::Number)
	{
		Rank = static_cast<int32>(JsonValue->AsNumber());
		OnRankFetchFinished.Broadcast(true, Rank);

		if (bHasPendingPredictedSubmission)
		{
			PendingPredictedSubmission.Rank = Rank;
			bHasPendingPredictedSubmission = false;
			OnPredictedSubmissionFetched.Broadcast(true, PendingPredictedSubmission);
		}
		return;
	}

	OnRankFetchFinished.Broadcast(false, -1);

	if (bHasPendingPredictedSubmission)
	{
		bHasPendingPredictedSubmission = false;
		OnPredictedSubmissionFetched.Broadcast(false, FLeaderboardSubmission());
	}
}

bool ULeaderboardService::BuildSubmissionFromBookStats(UBookStatsSubsystem* BookStatsSubsystem, FLeaderboardSubmission& OutSubmission) const
{
	if (!BookStatsSubsystem)
	{
		return false;
	}

	const FString FullText = BuildFullTextFromBookStats(BookStatsSubsystem);
	if (FullText.IsEmpty())
	{
		//return false;
	}

	OutSubmission = FLeaderboardSubmission();
	OutSubmission.AuthorName = BookStatsSubsystem->GetAuthorName().ToString();
	OutSubmission.BookTitle = BookStatsSubsystem->GetBookTitle().ToString();
	OutSubmission.Score = BookStatsSubsystem->GetTotalScore();
	OutSubmission.FullText = FullText;
	OutSubmission.PreviewText = FullText.Left(500);
	OutSubmission.AveragePerPage = BookStatsSubsystem->GetAverageScorePerPage();
	OutSubmission.PageCount = BookStatsSubsystem->GetPageCount();

	return true;
}

FString ULeaderboardService::BuildFullTextFromBookStats(const UBookStatsSubsystem* BookStatsSubsystem) const
{
	if (!BookStatsSubsystem)
	{
		return FString();
	}

	FString CombinedText;
	const TArray<FPageStats>& Pages = BookStatsSubsystem->GetAllPageStats();

	for (const FPageStats& Page : Pages)
	{
		const FString PageString = Page.PageText.ToString().TrimStartAndEnd();

		if (!PageString.IsEmpty())
		{
			if (!CombinedText.IsEmpty())
			{
				CombinedText += TEXT("\n\n");
			}

			CombinedText += PageString;
		}
	}

	return CombinedText;
}

void ULeaderboardService::SubmitBookStats(UBookStatsSubsystem* BookStatsSubsystem)
{
	FLeaderboardSubmission Submission;
	if (!BuildSubmissionFromBookStats(BookStatsSubsystem, Submission))
	{
		OnSubmitFinished.Broadcast(false, TEXT("Failed to build submission from book stats"));
		return;
	}

	SubmitSubmission(Submission);
}

bool ULeaderboardService::ParseSubmissionObject(const TSharedPtr<FJsonObject>& JsonObject, FLeaderboardSubmission& OutSubmission) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	OutSubmission = FLeaderboardSubmission();

	if (JsonObject->HasField(TEXT("id")))
	{
		OutSubmission.Id = static_cast<int64>(JsonObject->GetNumberField(TEXT("id")));
	}

	if (JsonObject->HasField(TEXT("username")))
	{
		OutSubmission.AuthorName = JsonObject->GetStringField(TEXT("username"));
	}

	if (JsonObject->HasField(TEXT("score")))
	{
		OutSubmission.Score = static_cast<int64>(JsonObject->GetNumberField(TEXT("score")));
	}

	if (JsonObject->HasField(TEXT("title")))
	{
		OutSubmission.BookTitle = JsonObject->GetStringField(TEXT("title"));
	}

	if (JsonObject->HasField(TEXT("preview_text")))
	{
		OutSubmission.PreviewText = JsonObject->GetStringField(TEXT("preview_text"));
	}

	if (JsonObject->HasField(TEXT("full_text")))
	{
		OutSubmission.FullText = JsonObject->GetStringField(TEXT("full_text"));
	}

	if (JsonObject->HasField(TEXT("avg_per_page")))
	{
		OutSubmission.AveragePerPage = static_cast<int32>(JsonObject->GetNumberField(TEXT("avg_per_page")));
	}

	if (JsonObject->HasField(TEXT("page_count")))
	{
		OutSubmission.PageCount = static_cast<int32>(JsonObject->GetNumberField(TEXT("page_count")));
	}

	if (JsonObject->HasField(TEXT("created_at")))
	{
		FString RawDate = JsonObject->GetStringField(TEXT("created_at"));

		FDateTime ParsedDate;
		if (FDateTime::ParseIso8601(*RawDate, ParsedDate))
		{
			OutSubmission.CreatedAt = ParsedDate.ToString(TEXT("%d/%m/%Y %H:%M"));
		}
		else
		{
			OutSubmission.CreatedAt = RawDate;
		}
	}

	if (JsonObject->HasField(TEXT("rank")))
	{
		OutSubmission.Rank = static_cast<int32>(JsonObject->GetNumberField(TEXT("rank")));
	}

	return true;
}

bool ULeaderboardService::ParseSubmissionArray(const FString& JsonString, TArray<FLeaderboardSubmission>& OutSubmissions) const
{
	OutSubmissions.Empty();

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonArray))
	{
		return false;
	}

	int32 RunningRank = 1;

	for (const TSharedPtr<FJsonValue>& Value : JsonArray)
	{
		if (!Value.IsValid() || Value->Type != EJson::Object)
		{
			continue;
		}

		FLeaderboardSubmission Submission;
		if (ParseSubmissionObject(Value->AsObject(), Submission))
		{
			// If the backend did not send a rank, and this array is from a top-list query,
			// array order is leaderboard order.
			if (Submission.Rank < 0)
			{
				Submission.Rank = RunningRank;
			}

			OutSubmissions.Add(MoveTemp(Submission));
			++RunningRank;
		}
	}

	return true;
}

bool ULeaderboardService::ParseSingleSubmission(const FString& JsonString, FLeaderboardSubmission& OutSubmission) const
{
	TArray<FLeaderboardSubmission> Submissions;
	if (!ParseSubmissionArray(JsonString, Submissions))
	{
		return false;
	}

	if (Submissions.IsEmpty())
	{
		return false;
	}

	OutSubmission = Submissions[0];
	return true;
}