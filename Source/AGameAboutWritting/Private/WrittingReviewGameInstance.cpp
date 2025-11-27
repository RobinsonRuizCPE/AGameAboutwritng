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
}