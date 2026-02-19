// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/UPostProcessStencilLibrary.h"
#include "PrimitiveSceneProxy.h"

static int32 ClampToStencilByte(int32 Mask)
{
	// CustomStencil is 0..255
	return FMath::Clamp(Mask, 0, 255);
}

void UPostProcessStencilLibrary::ApplyPostProcessEffects(
	UPrimitiveComponent* Primitive,
	int32 EffectsMask,
	bool bEnableCustomDepth
)
{
	if (!Primitive) return;

	if (bEnableCustomDepth)
	{
		Primitive->SetRenderCustomDepth(true);
	}

	const int32 StencilValue = ClampToStencilByte(EffectsMask);
	Primitive->SetCustomDepthStencilValue(StencilValue);
}

void UPostProcessStencilLibrary::AddPostProcessEffects(UPrimitiveComponent* Primitive, int32 EffectsToAdd)
{
	if (!Primitive) return;

	Primitive->SetRenderCustomDepth(true);
	const int32 Current = Primitive->CustomDepthStencilValue;
	const int32 NewValue = ClampToStencilByte(Current | EffectsToAdd);
	Primitive->SetCustomDepthStencilValue(NewValue);
}

void UPostProcessStencilLibrary::RemovePostProcessEffects(UPrimitiveComponent* Primitive, int32 EffectsToRemove)
{
	if (!Primitive) return;

	const int32 Current = Primitive->CustomDepthStencilValue;
	const int32 NewValue = ClampToStencilByte(Current & (~EffectsToRemove));
	Primitive->SetCustomDepthStencilValue(NewValue);
}

int32 UPostProcessStencilLibrary::GetPostProcessEffectsMask(UPrimitiveComponent* Primitive)
{
	if (!Primitive) return 0;
	return Primitive->CustomDepthStencilValue;
}