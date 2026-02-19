#pragma once

#include "CoreMinimal.h"
#include "PostProcessEffects.generated.h"

// Important: values MUST be powers of two
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class E_PostProcessEffects : uint8
{
	None       = 0 UMETA(Hidden),

	Pixelation = 1 << 0,
	Scanlines  = 1 << 1,

	// Add more later:
	// ChromaticAberration = 1 << 2,
	// Outline            = 1 << 3,
};

ENUM_CLASS_FLAGS(E_PostProcessEffects);