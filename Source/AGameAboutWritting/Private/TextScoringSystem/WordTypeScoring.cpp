// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/WordTypeScoring.h"

#include "WrittingReviewStruct.h"
#include "Engine/DataTable.h"

WordTypeScoring::WordTypeScoring(UDataTable* word_type_datatable)
{
	WordTypeMap.Empty();
	if (word_type_datatable)
	{
		for (const FName& RowName : word_type_datatable->GetRowNames())
		{
			if (const FWordTypeEntry* Row = word_type_datatable->FindRow<FWordTypeEntry>(RowName, TEXT("")))
			{
				WordTypeMap.Add(RowName.ToString().ToLower(), Row->Type);
			}
		}
	}
}

FString WordTypeScoring::GetWordType(FString const& word) const {
	if (const FString* Type = WordTypeMap.Find(word))
	{
		return *Type;
	}

	return "";
}

