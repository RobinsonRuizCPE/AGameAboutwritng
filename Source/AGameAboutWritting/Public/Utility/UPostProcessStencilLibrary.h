// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PostProcessEffects.h"
#include "UPostProcessStencilLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AGAMEABOUTWRITTING_API UPostProcessStencilLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Sets the stencil to exactly the provided effects
	UFUNCTION(BlueprintCallable, Category = "PostProcess|Stencil")
		static void ApplyPostProcessEffects(
			UPrimitiveComponent* Primitive,
			UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AGAMEABOUTWRITTING.E_PostProcessEffects"))
			int32 EffectsMask,
			bool bEnableCustomDepth = true
		);

	// Adds flags without removing existing ones
	UFUNCTION(BlueprintCallable, Category = "PostProcess|Stencil")
		static void AddPostProcessEffects(
			UPrimitiveComponent* Primitive,
			UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AGAMEABOUTWRITTING.E_PostProcessEffects"))
			int32 EffectsToAdd
		);

	// Removes flags while keeping others
	UFUNCTION(BlueprintCallable, Category = "PostProcess|Stencil")
		static void RemovePostProcessEffects(
			UPrimitiveComponent* Primitive,
			UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AGAMEABOUTWRITTING.E_PostProcessEffects"))
			int32 EffectsToRemove
		);

	UFUNCTION(BlueprintPure, Category = "PostProcess|Stencil")
		static int32 GetPostProcessEffectsMask(UPrimitiveComponent* Primitive);
};
