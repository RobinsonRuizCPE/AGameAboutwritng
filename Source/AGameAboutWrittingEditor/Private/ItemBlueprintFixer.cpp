#include "ItemBlueprintFixer.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"

void FItemBlueprintFixer::FixAllItemBlueprints()
{
    UE_LOG(LogTemp, Warning, TEXT("[FixItems] Start scanning /Game/Items"));

    FARFilter Filter;
    Filter.PackagePaths.Add("/Game/Items");
    Filter.bRecursivePaths = true;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());

    TArray<FAssetData> Assets;
    FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    ARM.Get().GetAssets(Filter, Assets);

    for (const FAssetData& AD : Assets)
    {
        UBlueprint* BP = Cast<UBlueprint>(AD.GetAsset());
        if (BP)
            FixBlueprint(BP);
    }

    UE_LOG(LogTemp, Warning, TEXT("[FixItems] DONE"));
}

void FItemBlueprintFixer::FixBlueprint(UBlueprint* BP)
{
    if (!BP) return;

    UE_LOG(LogTemp, Warning, TEXT("  Fixing %s"), *BP->GetName());

    USimpleConstructionScript* SCS = BP->SimpleConstructionScript;
    if (SCS)
    {
        // Remove every node
        TArray<USCS_Node*> Nodes = SCS->GetAllNodes();
        for (USCS_Node* Node : Nodes)
        {
            SCS->RemoveNode(Node);
        }
    }

    // Force blueprint to recreate hierarchy
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);

    // Compile
    FKismetEditorUtilities::CompileBlueprint(BP);

    UE_LOG(LogTemp, Warning, TEXT("  FIXED %s"), *BP->GetName());
}
