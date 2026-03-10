// Fill out your copyright notice in the Description page of Project Settings.

#include "WrittingReviewGameInstance.h"
#include "Engine/DataTable.h"

void UWrittingReviewGameInstance::Init()
{
	Super::Init();

	TextScoringSystem = NewObject<UTextScoringSystem>(this);
	PreviewManager = NewObject<UItemPreviewManager>(this);

	if (TextScoringSystem && WordFrequencyTable && WordTypeTable)
	{
		TextScoringSystem->Initialize(WordFrequencyTable, WordTypeTable, DTItemList);
	}

    LeaderboardService = NewObject<ULeaderboardService>(this);
    if (LeaderboardService)
    {
        LeaderboardService->Initialize(SupabaseProjectUrl, SupabaseAnonKey);
    }
}


void UWrittingReviewGameInstance::CreatePermanentWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		UE_LOG(LogTemp, Warning, TEXT("CreatePermanentWidget: Widget is invalid"));
		return;
	}

	UGameViewportClient* ViewportClient = GetGameViewportClient();
	if (!ViewportClient)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreatePermanentWidget: No viewport client"));
		return;
	}

	// If something old is still cached, clear it first.
	if (PermanentSlateWidget.IsValid())
	{
		ViewportClient->RemoveViewportWidgetContent(PermanentSlateWidget.ToSharedRef());
		PermanentSlateWidget.Reset();
		PermanentWidgetObject = nullptr;
	}

	PermanentWidgetObject = Widget;
	PermanentSlateWidget = Widget->TakeWidget();

	ViewportClient->AddViewportWidgetContent(PermanentSlateWidget.ToSharedRef(), 0);

	UE_LOG(LogTemp, Warning, TEXT("CreatePermanentWidget: Created and added %s"), *Widget->GetName());
}

void UWrittingReviewGameInstance::RemovePermanentWidget(UUserWidget* Widget)
{
	UGameViewportClient* ViewportClient = GetGameViewportClient();
	if (!ViewportClient)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemovePermanentWidget: No viewport client"));
		return;
	}

	if (PermanentSlateWidget.IsValid())
	{
		ViewportClient->RemoveViewportWidgetContent(PermanentSlateWidget.ToSharedRef());
	}

	if (PermanentWidgetObject)
	{
		PermanentWidgetObject->RemoveFromParent();
		UE_LOG(LogTemp, Warning, TEXT("RemovePermanentWidget: Removed and cleared %s"), *PermanentWidgetObject->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RemovePermanentWidget: Removed and cleared None"));
	}

	PermanentSlateWidget.Reset();
	PermanentWidgetObject = nullptr;
}

UMaterialInstanceDynamic* UWrittingReviewGameInstance::GetOrCreateMaterialInstance(
    UMaterialInterface* ParentMaterial,
    const FRatingPaperMaterialParameters& InParams)
{
    if (!ParentMaterial)
    {
        return nullptr;
    }

    const FRatingPaperMaterialParameters& Key = InParams;

    if (TObjectPtr<UMaterialInstanceDynamic>* FoundMID = CachedMIDs.Find(Key))
    {
        return FoundMID->Get();
    }

    UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(ParentMaterial, this);
    if (!NewMID)
    {
        return nullptr;
    }

    for (const FMaterialBoolParam& Param : Key.Params)
    {
        NewMID->SetScalarParameterValue(FName(*Param.Name), Param.Value ? 1.0f : 0.0f);
    }

    CachedMIDs.Add(Key, NewMID);
    UE_LOG(LogTemp, Warning, TEXT("Created MID %s Outer=%s"),
        *NewMID->GetName(),
        *GetNameSafe(NewMID->GetOuter()));
    return NewMID;
}