#pragma once

#include "CoreMinimal.h"

class UBlueprint;

class FItemBlueprintFixer
{
public:
    static void FixAllItemBlueprints();
    static void FixBlueprint(UBlueprint* BP);
};
