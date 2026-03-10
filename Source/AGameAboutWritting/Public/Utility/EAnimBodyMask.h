#pragma once

#include "CoreMinimal.h"
#include "EAnimBodyMask.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EAnimBodyMask : uint8
{
	None      = 0 UMETA(Hidden),

	RightArm  = 1 << 0,
	LeftArm   = 1 << 1,
	Head      = 1 << 2,
	Spine     = 1 << 3,
	Legs      = 1 << 4,

	// Convenience groups (optional)
	Arms      = RightArm | LeftArm,
	UpperBody = RightArm | LeftArm | Spine,
	FullBody  = 1 << 7,
};
ENUM_CLASS_FLAGS(EAnimBodyMask);

// Helpers (optional)
FORCEINLINE bool HasMask(int32 MaskBits, EAnimBodyMask Flag)
{
	return (MaskBits & static_cast<int32>(Flag)) != 0;
}