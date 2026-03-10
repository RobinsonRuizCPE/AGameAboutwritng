#pragma once

#include "CoreMinimal.h"
#include "../RatingSystem/RatingPaperMaterialParams.h"
#include "ESentenceStructAndTypes.generated.h"

UENUM(BlueprintType)
enum class ESentenceType : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	Dialog			UMETA(DisplayName = "Dialog"),
	Description		UMETA(DisplayName = "Description"),
	Question		UMETA(DisplayName = "Question"),
	Command			UMETA(DisplayName = "Command"),
	Exposition		UMETA(DisplayName = "Exposition"),
	InternalThought	UMETA(DisplayName = "InternalThought"),
	Narration		UMETA(DisplayName = "Narration"),
	Poetic			UMETA(DisplayName = "Poetic"),
	Statistical		UMETA(DisplayName = "Statistical"),
	Interjection	UMETA(DisplayName = "Interjection"),
};

UENUM(BlueprintType)
enum class ESentenceStructureType : uint8
{
	Unknown             UMETA(DisplayName = "Unknown"),
	Simple              UMETA(DisplayName = "Simple"),
	Compound            UMETA(DisplayName = "Compound"),
	Complex             UMETA(DisplayName = "Complex"),
	CompoundComplex     UMETA(DisplayName = "Compound-Complex"),
};


namespace SentenceStyle
{
	AGAMEABOUTWRITTING_API FLinearColor GetSentenceBgColor(ESentenceType Type);
	AGAMEABOUTWRITTING_API const FRatingPaperMaterialParameters& GetSentenceMaterialParameters(ESentenceStructureType Type);
}

UCLASS()
class AGAMEABOUTWRITTING_API USentenceStyleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Sentence Style")
	static FLinearColor GetSentenceBgColorBP(ESentenceType Type);

	UFUNCTION(BlueprintPure, Category="Sentence Style")
	static FRatingPaperMaterialParameters GetSentenceMaterialParametersBP(ESentenceStructureType Type);
};