// Fill out your copyright notice in the Description page of Project Settings.
#include "RatingSystem/DynamicTextRunDecorator.h"

#include "RatingSystem/FDynamicTextHighlightRun.h"
#include "RatingSystem/RuntimeMaterialRegistry.h"
#include "Framework/Text/SlateWidgetRun.h"
#include "RatingSystem/MyRichTextBlock.h"



class FDynamicTextRunDecorator : public FRichTextDecorator
{
public:
    FDynamicTextRunDecorator(URichTextBlock* InOwner)
        :FRichTextDecorator(InOwner), Owner(InOwner)
    {
    }

protected:

    virtual bool Supports(const FTextRunParseResults& RunInfo, const FString& Text) const override
    {
        auto test = RunInfo.Name.Equals(TEXT("dyn"), ESearchCase::IgnoreCase);
        return test;
    }

    virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& Style) const override
    {
        FLinearColor Bg = FLinearColor::Transparent;
        if (const FString* BgHex = RunInfo.MetaData.Find("bg"))
        {
            Bg = FLinearColor(FColor::FromHex(*BgHex));
        }

        FLinearColor TextColor = Style.ColorAndOpacity.GetSpecifiedColor();
        if (const FString* ColorHex = RunInfo.MetaData.Find("color"))
        {
            TextColor = FLinearColor(FColor::FromHex(*ColorHex));
        }

        int32 FontSize = Style.Font.Size;
        if (const FString* SizeStr = RunInfo.MetaData.Find("size"))
        {
            FontSize = FCString::Atoi(**SizeStr);
        }

        FSlateFontInfo FinalFont = Style.Font;
        FinalFont.Size = FontSize;
        FinalFont.FontMaterial = Style.Font.FontMaterial;

        // TEXT MATERIAL
        if (auto my_rich_text_block = Cast<UMyRichTextBlock>(Owner)) {
            UMaterialInterface* material_to_use = RunInfo.MetaData.Find("material") ? LoadObject<UMaterialInterface>(nullptr, **RunInfo.MetaData.Find("material")) : my_rich_text_block->GetBaseMaterial();
            if (auto DynMat = UMaterialInstanceDynamic::Create(material_to_use, my_rich_text_block)) {
                UE_LOG(LogTemp, Warning, TEXT("Created MID %s Outer=%s"),
                    *DynMat->GetName(),
                    *GetNameSafe(DynMat->GetOuter()));

                for (const auto& Elem : RunInfo.MetaData) {
                    const FString& Key = Elem.Key;
                    const FString& Value = Elem.Value;
                    if (Key.StartsWith("mat_param_"))
                    {
                        DynMat->SetScalarParameterValue(FName(Key.RightChop(strlen("mat_param_"))), FCString::Atof(*Value));
                    }
                }
                my_rich_text_block->RegisterTextMaterial(DynMat);
                FinalFont.FontMaterial = DynMat;
            }
        }
        
        return SNew(SBorder)
            .Padding(FMargin(0, 0))
            .BorderBackgroundColor(Bg)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            [
                SNew(STextBlock)
                .Text(FText::FromString(RunInfo.Content.ToString()))
            .ColorAndOpacity(TextColor)
            .Font(FinalFont)
            .WrapTextAt(0)
            ];
    }

private:
    URichTextBlock* Owner;
};

UDynamicTextDecorator::UDynamicTextDecorator(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

TSharedPtr<ITextDecorator> UDynamicTextDecorator::CreateDecorator(URichTextBlock* InOwner)
{
    return MakeShareable(new FDynamicTextRunDecorator(InOwner));
}
