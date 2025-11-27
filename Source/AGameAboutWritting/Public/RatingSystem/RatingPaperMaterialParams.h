// Fill out your copyright notice in the Description page of Project Settings.


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
        float Value = false;

    FMaterialBoolParam() {}

    FMaterialBoolParam(const FString& InName, float InValue)
        : Name(InName), Value(InValue)
    {}
};

USTRUCT(BlueprintType)
struct FRating_Paper_Material_Parameters
{
    GENERATED_BODY()

public:

    // Array of your param structs
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TArray<FMaterialBoolParam> Params;

    // Default constructor
    FRating_Paper_Material_Parameters() {}

    // Constructor that takes any number of params
    FRating_Paper_Material_Parameters(std::initializer_list<FMaterialBoolParam> InParams)
    {
        Params = InParams; // copies automatically
    }
};