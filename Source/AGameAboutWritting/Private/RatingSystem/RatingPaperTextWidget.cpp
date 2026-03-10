// Fill out your copyright notice in the Description page of Project Settings.


#include "RatingSystem/RatingPaperTextWidget.h"
#include "RatingSystem/DynamicTextRunDecorator.h"
#include "RatingSystem/FDynamicTextHighlightRun.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

void URatingPaperTextWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CustomTextBlock)
	{
		CustomTextBlock->SetAutoWrapText(true);
		CustomTextBlock->SetDecorators({ UDynamicTextDecorator::StaticClass() });
	}
}

void URatingPaperTextWidget::AddText(const FString& InText) {
	auto current_text = CustomTextBlock->GetText().ToString();
	auto const new_text = current_text.Append(InText);
	CustomTextBlock->SetText(FText::FromString(new_text));
}

void URatingPaperTextWidget::AddTextWithParams(
    const FString& InText,
    const FLinearColor& text_color,
    const FLinearColor& background_color,
    const FString& material_path,
    const FRatingPaperMaterialParameters& material_parameter)
{
    const FString HexColor = text_color.ToFColor(true).ToHex();
    const FString BgHex = background_color.ToFColor(true).ToHex();
    FString decorator_text = FString::Printf(TEXT("<dyn color=\"#%s\" bg=\"#%s\""), *HexColor, *BgHex);

    if (!material_path.IsEmpty())
    {
        decorator_text.Append(FString::Printf(TEXT(" material=\"%s\""), *material_path));
    }

    for (const auto& param : material_parameter.Params)
    {
        decorator_text.Append(FString::Printf(
            TEXT(" mat_param_%s=\"%d\""),
            *param.Name,
            param.Value ? 1 : 0));
    }

    decorator_text.Append(FString::Printf(TEXT(">%s</>"), *InText));

    FString current_text = CustomTextBlock->GetText().ToString();
    current_text.Append(decorator_text);
    CustomTextBlock->SetText(FText::FromString(current_text));
}

void URatingPaperTextWidget::SetTextWithParams(
    const FString& InText,
    const FLinearColor& text_color,
    const FLinearColor& background_color,
    const FString& material_path,
    const FRatingPaperMaterialParameters& material_parameter)
{
    ClearTextBlock();

    const FString HexColor = text_color.ToFColor(true).ToHex();
    const FString BgHex = background_color.ToFColor(true).ToHex();
    FString decorator_text = FString::Printf(TEXT("<dyn color=\"#%s\" bg=\"#%s\""), *HexColor, *BgHex);

    if (!material_path.IsEmpty())
    {
        decorator_text.Append(FString::Printf(TEXT(" material=\"%s\""), *material_path));
    }

    for (const auto& param : material_parameter.Params)
    {
        decorator_text.Append(FString::Printf(
            TEXT(" mat_param_%s=\"%d\""),
            *param.Name,
            param.Value ? 1 : 0));
    }

    decorator_text.Append(FString::Printf(TEXT(">%s</>"), *InText));
    CustomTextBlock->SetText(FText::FromString(decorator_text));
}

void URatingPaperTextWidget::ClearTextBlock() {
	if (CustomTextBlock->GetText().IsEmpty()) {
		return;
	}
	CustomTextBlock->SetText(FText::FromString(""));
	CustomTextBlock->ClearMaterials();
}

 
