// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "MyRichTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class AGAMEABOUTWRITTING_API UMyRichTextBlock : public URichTextBlock
{
	GENERATED_BODY()
public:

	UMaterialInterface* GetBaseMaterial() const { return BaseMaterial; }

	// Add a MID so it stays alive as long as the widget is alive
	void RegisterTextMaterial(UMaterialInstanceDynamic* Mat)
	{
		OwnedTextMaterials.Add(Mat);
	}

	void ClearMaterials()
	{
		OwnedTextMaterials.Empty();
	}

protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Widget")
		TArray<UMaterialInstanceDynamic*> OwnedTextMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paper Widget")
		UMaterialInterface* BaseMaterial;
};
