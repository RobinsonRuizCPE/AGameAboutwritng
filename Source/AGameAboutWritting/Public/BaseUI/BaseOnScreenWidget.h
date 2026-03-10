// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseOnScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class AGAMEABOUTWRITTING_API UBaseOnScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void BP_InitializeWidget();
};