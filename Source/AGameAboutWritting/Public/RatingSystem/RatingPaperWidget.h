// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RatingPaperTextWidget.h"
#include "TextScoringSystem/SentenceAnalyser.h"
#include "../Items/ItemTheme.h"
#include "RatingPaperMaterialParams.h"
#include "RatingPaperWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishedWrapUp);

UCLASS()
class AGAMEABOUTWRITTING_API URatingPaperWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
		void ScorePaperText(const FString& UserText);

	UFUNCTION(BlueprintCallable, Category = "Paper Widget")
		void SetFontInfo(const FSlateFontInfo& Font) { FontInfo = Font; CurrentTextWidget->GetTextBlock()->SetDefaultFont(FontInfo); }

	UFUNCTION(BlueprintCallable, Category = "Paper Widget")
		void AddWordToPage(const FString& WordText, const FLinearColor& Color);


	UFUNCTION(BlueprintCallable, Category = "Paper Widget")
		FLinearColor const GetSentenceTypeColor(ESentenceType sentence_type) const;

	UFUNCTION(BlueprintCallable, Category = "Paper Widget")
		FRating_Paper_Material_Parameters const& GetSentenceStructMaterialParameter(ESentenceStructureType sentence_struct) const;


	UFUNCTION(BlueprintImplementableEvent, Category = "Rating|Refresh")
		void RefreshSentenceStructCount(ESentenceStructureType Type);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rating|Refresh")
		void RefreshSentenceTypesCount(const TArray<ESentenceType>& SentenceTypes);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rating|Refresh")
		void HandleThemeFound(AItemTheme* item_them, FString const& word, int32 frequency_score ,float found_weight);

	UFUNCTION(BlueprintImplementableEvent, Category = "Rating|Refresh")
		void WrapUpRatingPaper();
	UFUNCTION(BlueprintCallable, Category = "Rating|Refresh")
		void RefreshWrapUpRatingPaper(){ WrapUpRatingPaper(); }

	UFUNCTION(BlueprintCallable)
		void TriggerFinishedWrapUp() { OnFinishedWrapUp.Broadcast(); }

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnFinishedWrapUp OnFinishedWrapUp;


private:
	UFUNCTION()
		void HandleWordProcessed(FString word, FLinearColor const color);

	UFUNCTION()
		void HandleSentenceProcessed(TArray<ESentenceType> sentence_types, ESentenceStructureType sentence_struct);

	UFUNCTION()
		void HandleScoreAdded(int32 ScoreToAdd, FString StringResponsible, FString Reason);

	UFUNCTION()
		void HandleReviewComplete();

	UFUNCTION()
		void FindThemeItems();

	UFUNCTION()
		void CheckWordTheme(FString const& word, int32 freq_score);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Paper Widget")
		UImage* BGImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Paper Widget")
		URatingPaperTextWidget* CurrentTextWidget;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Paper Widget")
		UVerticalBox* ItemThemeVerticalBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paper Widget")
		float WordPadding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Widget")
		TSubclassOf<URatingPaperTextWidget> TextBlockWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Widget")
	TMap<ESentenceType, uint8> SentenceTypesCountMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Widget")
	TMap<ESentenceStructureType, uint8> SentenceStructCountMap;

private:
	FSlateFontInfo FontInfo;

	FString PreviousWord = FString{};
	FLinearColor CurrentSentenceBgColor = FLinearColor{ 0,0,0,0.f };
	FRating_Paper_Material_Parameters CurrentmaterialParameters = {};

	FString CurrentMatInstancePath;
	int32 MaterialCounter = 0;

	TArray<AItemTheme*> CurrentItemThemeActors;
};


