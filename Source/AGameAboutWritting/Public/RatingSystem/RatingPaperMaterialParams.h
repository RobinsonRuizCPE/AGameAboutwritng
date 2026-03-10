#pragma once

#include "CoreMinimal.h"
#include "RatingPaperMaterialParams.generated.h"

USTRUCT(BlueprintType)
struct FMaterialBoolParam
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool Value = false;

    FMaterialBoolParam() = default;

    FMaterialBoolParam(const FString& InName, bool InValue)
        : Name(InName), Value(InValue)
    {
    }

    bool operator==(const FMaterialBoolParam& Other) const
    {
        return Name == Other.Name && Value == Other.Value;
    }

    friend uint32 GetTypeHash(const FMaterialBoolParam& Param)
    {
        return HashCombine(GetTypeHash(Param.Name), GetTypeHash(Param.Value));
    }
};

USTRUCT(BlueprintType)
struct FRatingPaperMaterialParameters
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FMaterialBoolParam> Params;

    FRatingPaperMaterialParameters() = default;

    FRatingPaperMaterialParameters(std::initializer_list<FMaterialBoolParam> InParams)
        : Params(InParams)
    {
    }

    bool operator==(const FRatingPaperMaterialParameters& Other) const
    {
        if (Params.Num() != Other.Params.Num())
        {
            return false;
        }

        for (int32 i = 0; i < Params.Num(); ++i)
        {
            if (!(Params[i] == Other.Params[i]))
            {
                return false;
            }
        }

        return true;
    }

    friend uint32 GetTypeHash(const FRatingPaperMaterialParameters& Parameters)
    {
        uint32 Hash = 0;
        for (const FMaterialBoolParam& Param : Parameters.Params)
        {
            Hash = HashCombine(Hash, GetTypeHash(Param));
        }
        return Hash;
    }
};