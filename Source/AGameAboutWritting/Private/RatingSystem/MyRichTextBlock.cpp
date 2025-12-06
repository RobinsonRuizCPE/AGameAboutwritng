// Fill out your copyright notice in the Description page of Project Settings.

#include "RatingSystem/MyRichTextBlock.h"

#include "RatingSystem/FDynamicTextHighlightRun.h"

void UMyRichTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
    UE_LOG(LogTemp, Warning, TEXT("UMyRichTextBlock::ReleaseSlate"));
    OwnedTextMaterials.Empty();
    const FText CurrentText = GetText();
    SetText(FText::GetEmpty());
    SetText(CurrentText);
    Super::ReleaseSlateResources(bReleaseChildren);
}

void UMyRichTextBlock::BeginDestroy()
{
    UE_LOG(LogTemp, Warning, TEXT("UMyRichTextBlock::BeginDestroy"));
    OwnedTextMaterials.Empty();
    Super::BeginDestroy();
}

void UMyRichTextBlock::SynchronizeProperties()
{
    UE_LOG(LogTemp, Warning, TEXT("UMyRichTextBlock::SyncProperties"));
    Super::SynchronizeProperties();
    // rebuild underlying SWidget, which invalidates runs
}

TSharedRef<SWidget> UMyRichTextBlock::RebuildWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("UMyRichTextBlock::RebuildWidget"));

    ClearMaterials();

    // Force rebuild of Slate text runs by clearing the text
    const FText CurrentText = GetText();
    Super::SetText(FText::GetEmpty());

    // Mark this widget as volatile to force a redraw
    this->SetRenderTransform(FWidgetTransform());
    this->ForceLayoutPrepass();

    // Fully rebuild the underlying Slate tree
    TSharedRef<SWidget> Widget = Super::RebuildWidget();

    // Restore the text (forces a new FSlateTextLayout)
    Super::SetText(CurrentText);

    return Widget;
}
