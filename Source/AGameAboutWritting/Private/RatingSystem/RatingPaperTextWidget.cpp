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

void URatingPaperTextWidget::AddTextWithParams(const FString& InText, FLinearColor const& text_color, FLinearColor const& background_color, FString const& material_path, FRating_Paper_Material_Parameters const& material_parameter) {
	FString HexColor = text_color.ToFColor(true).ToHex();
	FString BgHex = background_color.ToFColor(true).ToHex();
	FString decorator_text = FString::Printf(TEXT("<dyn color=\"#%s\" bg=\"#%s\""), *HexColor, *BgHex);

	//Mat path
	if (!material_path.IsEmpty()) {
		decorator_text.Append(FString::Printf(TEXT(" material=\"%s\""), *material_path));
	}

	//Mat params
	for (auto const& param : material_parameter.Params) {
		decorator_text.Append(FString::Printf(TEXT(" mat_param_%s=\"%f\""), *param.Name, param.Value));
	}

	// Text
	decorator_text.Append(FString::Printf(TEXT(">%s</>"), *InText));
	auto current_text = CustomTextBlock->GetText().ToString();
	auto const new_text = current_text.Append(decorator_text);
	CustomTextBlock->SetText(FText::FromString(new_text));
}

void URatingPaperTextWidget::SetTextWithParams(const FString& InText, FLinearColor const& text_color, FLinearColor const& background_color, FString const& material_path, FRating_Paper_Material_Parameters const& material_parameter) {

	ClearTextBlock();

	// Color
	FString HexColor = text_color.ToFColor(true).ToHex();
	FString BgHex = background_color.ToFColor(true).ToHex();
	FString decorator_text = FString::Printf(TEXT("<dyn color=\"#%s\" bg=\"#%s\""), *HexColor, *BgHex);

	//Mat path
	if (!material_path.IsEmpty()) {
		decorator_text.Append(FString::Printf(TEXT(" material=\"%s\""), *material_path));
	}

	//Mat params
	for (auto const& param : material_parameter.Params) {
		decorator_text.Append(FString::Printf(TEXT(" mat_param_%s=\"%f\""), *param.Name, param.Value));
	}

	// Text
	decorator_text.Append(FString::Printf(TEXT(">%s</>"), *InText));
	CustomTextBlock->InvalidateLayoutAndVolatility();
	CustomTextBlock->GetCachedWidget()->Invalidate(EInvalidateWidget::PaintAndVolatility);
	CustomTextBlock->SetText(FText::FromString(decorator_text));
}

void URatingPaperTextWidget::ClearTextBlock() {
	if (CustomTextBlock->GetText().IsEmpty()) {
		return;
	}
	CustomTextBlock->SetText(FText::FromString(""));
	CustomTextBlock->ClearMaterials();
}

 
