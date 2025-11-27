// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "Components/RichTextBlockDecorator.h"

#include "Framework/Text/SlateTextRun.h"
#include "Framework/Text/SlateTextLayout.h"
#include "Framework/Text/TextLineHighlight.h"
#include "Fonts/SlateFontInfo.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Optional.h"
#include "Framework/Text/RichTextLayoutMarshaller.h"

#include "DynamicTextRunDecorator.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class AGAMEABOUTWRITTING_API UDynamicTextDecorator : public URichTextBlockDecorator
{
    GENERATED_BODY()
public:
    UDynamicTextDecorator(const FObjectInitializer& ObjectInitializer);

protected:
    virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};