#include "Utility/ESentenceStructAndTypes.h"

namespace
{
	const TMap<ESentenceType, FLinearColor> GSentenceBgColors =
	{
		{ ESentenceType::Unknown,			FLinearColor(1.f, 1.f, 1.f, 1.f) },
		{ ESentenceType::Dialog,			FLinearColor(1.f, 0.f, 0.f, 0.5f) },
		{ ESentenceType::Description,		FLinearColor(1.f, 1.f, 0.f, 0.5f) },
		{ ESentenceType::Question,			FLinearColor(1.f, 1.f, 1.f, 0.5f) },
		{ ESentenceType::Command,			FLinearColor(0.f, 1.f, 0.f, 0.5f) },
		{ ESentenceType::Exposition,		FLinearColor(0.f, 1.f, 1.f, 0.5f) },
		{ ESentenceType::InternalThought,	FLinearColor(0.f, 0.f, 1.f, 0.5f) },
		{ ESentenceType::Narration,			FLinearColor(1.f, 0.f, 1.f, 0.5f) },
		{ ESentenceType::Poetic,			FLinearColor(1.f, 0.5f, 0.25f, 0.5f) },
		{ ESentenceType::Statistical,		FLinearColor(0.25f, 0.8f, 0.4f, 0.5f) },
		{ ESentenceType::Interjection,		FLinearColor(0.5f, 0.5f, 0.9f, 0.5f) }
	};

	const FRatingPaperMaterialParameters GDefaultSentenceMaterialParams =
	{
		{ TEXT("Waving"), false },
		{ TEXT("Breathing"), false },
		{ TEXT("CustomTextureLerpValue"), false }
	};

	const TMap<ESentenceStructureType, FRatingPaperMaterialParameters> GSentenceMaterialParameters =
	{
		{ ESentenceStructureType::Unknown,			FRatingPaperMaterialParameters{ { TEXT("Waving"), false }, { TEXT("Breathing"), false }, { TEXT("CustomTextureLerpValue"), false } } },
		{ ESentenceStructureType::Simple,			FRatingPaperMaterialParameters{ { TEXT("Waving"), false }, { TEXT("Breathing"), false }, { TEXT("CustomTextureLerpValue"), false } } },
		{ ESentenceStructureType::Compound,			FRatingPaperMaterialParameters{ { TEXT("Waving"), true  }, { TEXT("Breathing"), false }, { TEXT("CustomTextureLerpValue"), false } } },
		{ ESentenceStructureType::Complex,			FRatingPaperMaterialParameters{ { TEXT("Waving"), false }, { TEXT("Breathing"), true  }, { TEXT("CustomTextureLerpValue"), false } } },
		{ ESentenceStructureType::CompoundComplex,	FRatingPaperMaterialParameters{ { TEXT("Waving"), false }, { TEXT("Breathing"), false }, { TEXT("CustomTextureLerpValue"), true  } } }
	};
}

FLinearColor SentenceStyle::GetSentenceBgColor(ESentenceType Type)
{
	if (const FLinearColor* Found = GSentenceBgColors.Find(Type))
	{
		return *Found;
	}

	return FLinearColor::Transparent;
}

const FRatingPaperMaterialParameters& SentenceStyle::GetSentenceMaterialParameters(ESentenceStructureType Type)
{
	if (const FRatingPaperMaterialParameters* Found = GSentenceMaterialParameters.Find(Type))
	{
		return *Found;
	}

	return GDefaultSentenceMaterialParams;
}

FLinearColor USentenceStyleLibrary::GetSentenceBgColorBP(ESentenceType Type)
{
	return SentenceStyle::GetSentenceBgColor(Type);
}

FRatingPaperMaterialParameters USentenceStyleLibrary::GetSentenceMaterialParametersBP(ESentenceStructureType Type)
{
	return SentenceStyle::GetSentenceMaterialParameters(Type);
}