// Fill out your copyright notice in the Description page of Project Settings.


#include "RatingSystem/RatingPaperTextWidget.h"
#include "RatingSystem/DynamicTextRunDecorator.h"
#include "RatingSystem/FDynamicTextHighlightRun.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

void URatingPaperTextWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (TextBlock)
	{
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetDecorators({ UDynamicTextDecorator::StaticClass() });
	}
}


void URatingPaperTextWidget::SetupText(const FString& InText, const FLinearColor& InColor, const FSlateFontInfo& InFont)
{
	if (!TextBlock) return;

	TextBlock->SetText(FText::FromString(InText));
	TextBlock->SetDefaultColorAndOpacity(FSlateColor(InColor));
	TextBlock->SetDefaultFont(InFont);
}

void URatingPaperTextWidget::SetHighlightColor(FLinearColor const& InColor)
{
	//BGBorder->SetBrushColor(InColor);
}

void URatingPaperTextWidget::SetBackgroundEffect()
{
	//BGBorder->SetBrushColor(FLinearColor{1,1,1,1});

}

void URatingPaperTextWidget::AddText(const FString& InText) {
	auto current_text = TextBlock->GetText().ToString();
	auto const new_text = current_text.Append(InText);
	TextBlock->SetText(FText::FromString(new_text));
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
	auto current_text = TextBlock->GetText().ToString();
	auto const new_text = current_text.Append(decorator_text);
	TextBlock->SetText(FText::FromString(new_text));
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
	TextBlock->SetText(FText::FromString(decorator_text));
}

void URatingPaperTextWidget::ClearTextBlock() {
	if (TextBlock->GetText().IsEmpty()) {
		return;
	}
	TextBlock->SetText(FText::FromString(""));
	TextBlock->ClearMaterials();
}

 
