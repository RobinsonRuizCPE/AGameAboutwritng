// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePaperWidget.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Engine/DataTable.h"
#include "UObject/UnrealType.h"
#include "WrittingReviewGameInstance.h"

void UBasePaperWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Example: Set placeholder text and tint the image (optional logic)
	if (MultiLineEditableText_0)
	{
		MultiLineEditableText_0->SetText(FText::FromString(TEXT("Start typing...")));
		MultiLineEditableText_0->OnTextChanged.AddDynamic(this, &UBasePaperWidget::HandleTextChanged);
		PreviousActualLines = 1;
		PreviousEstimatedLines = 1;

		PreviousValidText = MultiLineEditableText_0->GetText();

		CachedLineHeight = MultiLineEditableText_0->GetFont().Size;
	}

	if (BGImage)
	{
		BGImage->SetOpacity(1.f);  // Just an example of modifying the image
	}
}

void UBasePaperWidget::HandleTextChanged(const FText& NewText)
{
	FString CurrentText = NewText.ToString();
	int32 ActualNewlines = CountActualNewlines(CurrentText);
	int32 EstimatedLines = EstimateWrappedLineCount(CurrentText);
	int32 TotalLines = ActualNewlines + EstimatedLines;
	if (TotalLines > MaxLinesPerPage)
	{
		// Restore to previous valid text
		if (MultiLineEditableText_0)
		{
			MultiLineEditableText_0->SetText(PreviousValidText);
		}
		ComputedText = FText::FromString(ComputeWrappedText(PreviousValidText.ToString()));
		OnPageFull.Broadcast(ComputedText);
		return;
	}

	// Update last valid text
	PreviousValidText = NewText;

	bool bLineCountChanged = false;

	if (ActualNewlines != PreviousActualLines)
	{
		int32 Delta = ActualNewlines - PreviousActualLines;
		FColor Color = Delta > 0 ? FColor::Green : FColor::Red;
		bLineCountChanged = true;
	}

	if (EstimatedLines != PreviousEstimatedLines)
	{
		bLineCountChanged = true;
	}

	if (bLineCountChanged)
	{
		int32 CombinedPrevious = FMath::Max(PreviousActualLines, PreviousEstimatedLines);
		int32 CombinedCurrent = FMath::Max(ActualNewlines, EstimatedLines);
		OnLineCountChanged.Broadcast(CombinedCurrent, CombinedPrevious, CachedLineHeight);
	}

	PreviousActualLines = ActualNewlines;
	PreviousEstimatedLines = EstimatedLines;
}
int32 UBasePaperWidget::CountActualNewlines(const FString& Text) const
{
	// Count '\n' + 1 (since even empty or one-line has 1 logical line)
	int32 Count = 1;
	for (TCHAR Char : Text)
	{
		if (Char == '\n')
		{
			Count++;
		}
	}
	return Count;
}

int32 UBasePaperWidget::EstimateWrappedLineCount(const FString& Text) const
{
	if (!MultiLineEditableText_0 || !MultiLineEditableText_0->GetCachedWidget().IsValid())
		return 0;
	auto const words_token = GetListOfWords(Text);
	auto const number_of_lines_estimated = GetEstimatedLineCount(words_token);
	return number_of_lines_estimated;
}

TArray<FString> UBasePaperWidget::GetListOfWords(const FString& Text) const {
	TArray<FString> Tokens;
	FString CurrentToken;
	bool bLastWasSpace = false;
	const FString TextStr = Text;

	for (TCHAR Char : TextStr)
	{
		bool bIsSpace = FChar::IsWhitespace(Char);

		if (bIsSpace)
		{
			if (!bLastWasSpace && !CurrentToken.IsEmpty())
			{
				Tokens.Add(CurrentToken);
				CurrentToken.Empty();
			}
			CurrentToken.AppendChar(Char);
			bLastWasSpace = true;
		}
		else
		{
			if (bLastWasSpace && !CurrentToken.IsEmpty())
			{
				Tokens.Add(CurrentToken);
				CurrentToken.Empty();
			}
			CurrentToken.AppendChar(Char);
			bLastWasSpace = false;
		}
	}

	if (!CurrentToken.IsEmpty())
	{
		Tokens.Add(CurrentToken);
	}

	return Tokens;
}

int32 UBasePaperWidget::GetEstimatedLineCount(TArray<FString> const& world_list) const {
	int32 LineCount = 1;
	float CurrentLineWidth = 0.0f;

	// Get the font and box width
	FSlateFontInfo FontInfo = MultiLineEditableText_0->GetFont();
	TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	// Get box width
	FGeometry Geometry = MultiLineEditableText_0->GetCachedGeometry();

	float BoxWidth = Geometry.GetLocalSize().X;
	for (const FString& Token : world_list)
	{
		FVector2D TokenSize = FontMeasure->Measure(Token, FontInfo);
		float TokenWidth = TokenSize.X;

		if (CurrentLineWidth + TokenWidth > BoxWidth)
		{
			LineCount++;
			CurrentLineWidth = TokenWidth;
		}
		else
		{
			CurrentLineWidth += TokenWidth;
		}
	}

	return LineCount;
}

FString UBasePaperWidget::ComputeWrappedText(const FString& Source) const
{
	if (!MultiLineEditableText_0 || !MultiLineEditableText_0->GetCachedWidget().IsValid())
	{
		return Source; // fallback: no widget geometry / no font -> no tagging
	}

	const FSlateFontInfo FontInfo = MultiLineEditableText_0->GetFont();
	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const float BoxWidth =
		MultiLineEditableText_0->GetCachedGeometry().GetLocalSize().X;

	FString Result;
	Result.Reserve(Source.Len() + 32); // small optimization

	float CurrentLineWidth = 0.0f;

	const int32 Len = Source.Len();
	int32 i = 0;

	while (i < Len)
	{
		const TCHAR C = Source[i];

		// --- 1) Handle explicit newlines (\r, \n, \r\n) as *hard* breaks ---
		if (C == '\r' || C == '\n')
		{
			// handle CRLF pair
			if (C == '\r' && i + 1 < Len && Source[i + 1] == '\n')
			{
				Result.AppendChar('\r');
				Result.AppendChar('\n');
				i += 2;
			}
			else
			{
				Result.AppendChar(C);
				++i;
			}

			// reset line width after a real newline
			CurrentLineWidth = 0.0f;
			continue;
		}

		// --- 2) Build the next "token" (word or run of spaces), stopping before newline ---
		const int32 TokenStart = i;
		while (i < Len)
		{
			const TCHAR D = Source[i];

			if (D == '\r' || D == '\n')
			{
				// don't consume newline here; outer loop will handle it
				break;
			}

			// We define token as sequence of either all-space or all-non-space
			if (i > TokenStart)
			{
				const bool bPrevIsSpace = FChar::IsWhitespace(Source[i - 1]);
				const bool bCurIsSpace = FChar::IsWhitespace(D);
				if (bPrevIsSpace != bCurIsSpace)
				{
					break;
				}
			}

			++i;
		}

		const int32 TokenLen = i - TokenStart;
		if (TokenLen <= 0)
		{
			// Shouldn’t normally happen, but guard anyway.
			continue;
		}

		const FString Token = Source.Mid(TokenStart, TokenLen);

		// --- 3) Measure token width ---
		const float TokenWidth = FontMeasure->Measure(Token, FontInfo).X;

		// --- 4) If adding this token would overflow, we insert a wrap marker ---
		if (CurrentLineWidth > 0.0f && CurrentLineWidth + TokenWidth > BoxWidth)
		{
			// Insert wrap marker *before* starting this new token
			Result += TEXT("\\w");
			CurrentLineWidth = 0.0f;
		}

		// --- 5) Append token and accumulate width ---
		Result += Token;
		CurrentLineWidth += TokenWidth;
	}

	return Result;
}