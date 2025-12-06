// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "MyRichTextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "RatingPaperMaterialParams.h"
#include "RatingPaperTextWidget.generated.h"


UCLASS()
class AGAMEABOUTWRITTING_API URatingPaperTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		void AddText(const FString& InText);

	UFUNCTION(BlueprintCallable)
		void AddTextWithParams(const FString& InText, FLinearColor const& text_color, FLinearColor const& background_color, FString const& material_path, FRating_Paper_Material_Parameters const& material_parameter);

	UFUNCTION(BlueprintCallable)
		void SetTextWithParams(const FString& InText, FLinearColor const& text_color, FLinearColor const& background_color, FString const& material_path, FRating_Paper_Material_Parameters const& material_parameter);

	UFUNCTION(BlueprintCallable)
		void ClearTextBlock();

	UFUNCTION(BlueprintCallable)
	URichTextBlock* GetTextBlock() const { return CustomTextBlock; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
		UMyRichTextBlock* CustomTextBlock;
};