// WordFrequencyEntry.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WrittingReviewStruct.generated.h"

USTRUCT(BlueprintType)
struct AGAMEABOUTWRITTING_API FWordFrequencyEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int64 Count = 0;
};

USTRUCT(BlueprintType)
struct AGAMEABOUTWRITTING_API FWordTypeEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Type = "unknown";
};