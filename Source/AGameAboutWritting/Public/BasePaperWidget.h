#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/MultiLineEditableText.h"
#include "Components/Image.h"
#include "BasePaperWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnLineCountChanged,
	int32, NewLineCount,
	int32, PreviousLineCount,
	float, LineHeight
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPageFull,
	FText, TextToAnalyse
);

UCLASS()
class AGAMEABOUTWRITTING_API UBasePaperWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLineCountChanged OnLineCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPageFull OnPageFull;

private:
	UFUNCTION()
	void HandleTextChanged(const FText& NewText);

	int32 CountActualNewlines(const FString& Text) const;
	int32 EstimateWrappedLineCount(const FString& Text) const;

	TArray<FString> GetListOfWords(const FString& Text) const;
	int32 GetEstimatedLineCount(TArray<FString> const& world_list) const;

	FString ComputeWrappedText(const FString& Text) const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Paper Widget")
		UMultiLineEditableText* MultiLineEditableText_0;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Paper Widget")
		UImage* BGImage;

	UPROPERTY(BlueprintReadWrite, Category = "Paper Widget")
	int32 MaxLinesPerPage = 14;

private:
	int32 PreviousActualLines = 0;
	int32 PreviousEstimatedLines = 0;
	float CachedLineHeight = 0.0f;

	FText PreviousValidText;
	FText ComputedText;
};