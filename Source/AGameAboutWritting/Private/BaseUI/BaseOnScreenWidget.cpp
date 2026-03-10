// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUI/BaseOnScreenWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

void UBaseOnScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	BP_InitializeWidget();
}