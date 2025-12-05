// Fill out your copyright notice in the Description page of Project Settings.


#include "TextScoringSystem/ItemListFinder.h"

static void* CN_DllHandle = nullptr;
typedef int (*GetRelatedFunc)(const char*, RelatedResult**);
typedef void (*FreeResultsFunc)(RelatedResult*, int);

GetRelatedFunc GetRelatedWords = nullptr;
FreeResultsFunc FreeResults = nullptr;

ItemListFinder::ItemListFinder(UDataTable* item_list_data_table)
{
    FString DllPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/ConceptRelationDll.dll"));
    CN_DllHandle = FPlatformProcess::GetDllHandle(*DllPath);

    if (CN_DllHandle)
    {
        GetRelatedWords = (GetRelatedFunc)FPlatformProcess::GetDllExport(CN_DllHandle, TEXT("GetRelated"));
        FreeResults = (FreeResultsFunc)FPlatformProcess::GetDllExport(CN_DllHandle, TEXT("FreeResults"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DLL."));
    }

    WordToItemClassMap.Empty();
    if (!item_list_data_table)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemListFinder: DataTable is null!"));
        return;
    }

    const UScriptStruct* RowStruct = item_list_data_table->GetRowStruct();
    if (!RowStruct)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemListFinder: DataTable has no valid RowStruct!"));
        return;
    }

    static const FString ContextString(TEXT("ItemListFinderContext"));

    // --- Locate properties safely, even if it's a Blueprint struct ---
    FArrayProperty* TextArrayProp = nullptr;
    FObjectPropertyBase* ItemClassProp = nullptr;

    for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
    {
        FProperty* Prop = *It;
        const FString PropName = Prop->GetName();

        if (!TextArrayProp && PropName.StartsWith(TEXT("TextToRecognise"), ESearchCase::IgnoreCase))
        {
            TextArrayProp = CastField<FArrayProperty>(Prop);
        }
        else if (!ItemClassProp && PropName.StartsWith(TEXT("ItemClass"), ESearchCase::IgnoreCase))
        {
            ItemClassProp = CastField<FObjectPropertyBase>(Prop);
        }
    }

    if (!TextArrayProp || !ItemClassProp)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemListFinder: Could not find TextToRecognise or ItemClass properties!"));
        UE_LOG(LogTemp, Warning, TEXT("Listing found properties:"));
        for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
        {
            UE_LOG(LogTemp, Warning, TEXT(" - %s (%s)"), *It->GetName(), *It->GetClass()->GetName());
        }
        return;
    }

    // --- Iterate through all DataTable rows ---
    TArray<FName> RowNames = item_list_data_table->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        uint8* RowData = item_list_data_table->FindRowUnchecked(RowName);
        if (!RowData)
            continue;

        // Get TextToRecognise array
        FScriptArrayHelper ArrayHelper(TextArrayProp, TextArrayProp->ContainerPtrToValuePtr<void>(RowData));

        // Get ItemClass value (works for FClassProperty, FSoftClassProperty, etc.)
        UClass* ItemClass = Cast<UClass>(ItemClassProp->GetObjectPropertyValue_InContainer(RowData));

        FProperty* InnerProp = TextArrayProp->Inner;

        for (int32 i = 0; i < ArrayHelper.Num(); ++i)
        {
            void* ElementPtr = ArrayHelper.GetRawPtr(i);
            FString WordValue;

            if (FTextProperty* TextProp = CastField<FTextProperty>(InnerProp))
            {
                FText TextValue = TextProp->GetPropertyValue(ElementPtr);
                WordValue = TextValue.ToString();
            }
            else if (FStrProperty* StrProp = CastField<FStrProperty>(InnerProp))
            {
                // fallback if it’s actually a string array
                WordValue = StrProp->GetPropertyValue(ElementPtr);
            }

            if (!WordValue.IsEmpty())
            {
                FString LowerWord = WordValue.ToLower();
                ItemToGeneralThemeMap.FindOrAdd(ItemClass).Add(LowerWord);
                WordToItemClassMap.Add(LowerWord, ItemClass);
                if (GetRelatedWords) {
                    FTCHARToUTF8 WordUtf8(*LowerWord);

                    RelatedResult* Results = nullptr;
                    int Count = GetRelatedWords(WordUtf8.Get(), &Results);

                    TMap<FString, float> RelatedMap;
                    RelatedMap.Add(LowerWord, 2.0);

                    for (int K = 0; K < Count; K++)
                    {
                        FString Label = UTF8_TO_TCHAR(Results[K].label);
                        float Weight = Results[K].weight;

                        RelatedMap.Add(Label, Weight + 1);
                        WordToItemClassMap.Add(Label, ItemClass);
                    }

                    WordRelatedWeights.Add(LowerWord, RelatedMap);
                    FreeResults(Results, Count);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ItemListFinder: Parsed %d entries."), WordToItemClassMap.Num());
}

ItemListFinder::~ItemListFinder()
{
}

TArray<UClass*> ItemListFinder::GetCorrespondingItemClasses(FString const& word)
{
    FString LowerWord = word.ToLower();
    TArray<UClass*> array_to_return;
    WordToItemClassMap.MultiFind(LowerWord, array_to_return);
    return array_to_return;
}


TMap<FString, float> ItemListFinder::GetWordRelatedWeights(UClass* class_to_search) {
    TMap<FString, float> map_to_return;
    if (auto found_class = ItemToGeneralThemeMap.Find(class_to_search)) {
        for (auto item : *found_class) {
            if (auto found_weights = WordRelatedWeights.Find(item)) {
                map_to_return.Append(*found_weights);
            }
        }
    }

    return map_to_return;
}

TSet<FString> ItemListFinder::GetBaseThemeFromClass(UClass* class_to_search) {
    if (auto found_class = ItemToGeneralThemeMap.Find(class_to_search)) {
        return TSet<FString>{*found_class};
    }

    return {};
}


