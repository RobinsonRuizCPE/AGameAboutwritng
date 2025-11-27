#include "RatingSystem/RatingPaperWidget.h"
#include "WrittingReviewGameInstance.h"
#include "Components/WrapBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "RatingSystem/RuntimeMaterialRegistry.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Components/TextBlock.h"
#include "ThemeElements/ThemeHolder.h"
#include "Kismet/GameplayStatics.h"

static const TMap<ESentenceType, FLinearColor> SentenceBgColor = {
	{ ESentenceType::Unknown			,  FLinearColor{0,0,0,0.f}		},
	{ ESentenceType::Dialog				,  FLinearColor{1,0,0,0.5f}		},
	{ ESentenceType::Description		,  FLinearColor{1,1,0,0.5f}		},
	{ ESentenceType::Question			,  FLinearColor{1,1,1,0.5f}		},
	{ ESentenceType::Command			,  FLinearColor{0,1,0,0.5f}		},
	{ ESentenceType::Exposition			,  FLinearColor{0,1,1,0.5f}		},
	{ ESentenceType::InternalThought	,  FLinearColor{0,0,1,0.5f}		},
	{ ESentenceType::Narration			,  FLinearColor{1,0,1,0.5f}		},
	{ ESentenceType::Poetic				,  FLinearColor{1,0.5,0.25,0.5f}	},
	{ ESentenceType::Statistical		,  FLinearColor{0.25,0.8,0.4,0.5f}},
	{ ESentenceType::Interjection		,  FLinearColor{0.5,0.5,0.9,0.5f}}
};

static const TMap<ESentenceStructureType, FRating_Paper_Material_Parameters> SentenceMaterialParameter = {
	{ ESentenceStructureType::Unknown			,FRating_Paper_Material_Parameters{{"Waving",false}, {"Breathing",false}, {"CustomTextureLerpValue",false}}  },
	{ ESentenceStructureType::Simple         	,FRating_Paper_Material_Parameters{{"Waving",false}, {"Breathing",false}, {"CustomTextureLerpValue",false}}  },
	{ ESentenceStructureType::Compound       	,FRating_Paper_Material_Parameters{{"Waving",true} , {"Breathing",false}, {"CustomTextureLerpValue",false} }  },
	{ ESentenceStructureType::Complex        	,FRating_Paper_Material_Parameters{{"Waving",false}, {"Breathing",true }, {"CustomTextureLerpValue",false} }  },
	{ ESentenceStructureType::CompoundComplex	,FRating_Paper_Material_Parameters{{"Waving",false}, {"Breathing",false}, {"CustomTextureLerpValue",true }}  }
};

FLinearColor const URatingPaperWidget::GetSentenceTypeColor(ESentenceType sentence_type) const {
	if (sentence_type == ESentenceType::Unknown) {
		return FLinearColor{ 1,1,1,1.f };
	}
	auto color = SentenceBgColor.Find(sentence_type);
	return color ? *color : FLinearColor{ 0,0,0,0.f };
}

FRating_Paper_Material_Parameters const& URatingPaperWidget::GetSentenceStructMaterialParameter(ESentenceStructureType sentence_struct) const {
	const FRating_Paper_Material_Parameters* Result = SentenceMaterialParameter.Find(sentence_struct);
	if (!Result) {
		static FRating_Paper_Material_Parameters Dummy{};
		return Dummy;
	}
	return *Result;
}

void URatingPaperWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BGImage)
	{
		BGImage->SetOpacity(1.f);
	}

	UWrittingReviewGameInstance* GI = Cast<UWrittingReviewGameInstance>(GetGameInstance());
	if (GI && GI->TextScoringSystem)
	{
		GI->TextScoringSystem->OnScoreAdded.AddDynamic(this, &URatingPaperWidget::HandleScoreAdded);
		GI->TextScoringSystem->OnWordProcessed.AddDynamic(this, &URatingPaperWidget::HandleWordProcessed);
		GI->TextScoringSystem->OnSentencedProcessed.AddDynamic(this, &URatingPaperWidget::HandleSentenceProcessed);
	}


	FindThemeItems();
}

void URatingPaperWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Prevent CDO contamination
	if (IsDesignTime() || HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
}

void URatingPaperWidget::NativeDestruct()
{
	if (UWrittingReviewGameInstance* GI = Cast<UWrittingReviewGameInstance>(GetGameInstance()))
	{
		if (GI->TextScoringSystem)
		{
			GI->TextScoringSystem->OnScoreAdded.RemoveDynamic(this, &URatingPaperWidget::HandleScoreAdded);
			GI->TextScoringSystem->OnWordProcessed.RemoveDynamic(this, &URatingPaperWidget::HandleWordProcessed);
			GI->TextScoringSystem->OnSentencedProcessed.RemoveDynamic(this, &URatingPaperWidget::HandleSentenceProcessed);
		}
	}

	CurrentItemThemeActors.Empty();

	Super::NativeDestruct();
}

void URatingPaperWidget::AddWordToPage(const FString& WordText, const FLinearColor& Color)
{
	if (WordText.IsEmpty())
	{
		return;
	}

	if (PreviousWord == "\n" && WordText == " ") {
		return;
	}

	FString HexColor = Color.ToFColor(true).ToHex();
	FString BgHex = CurrentSentenceBgColor.ToFColor(true).ToHex();

	FString Styled = FString::Printf(
		TEXT("<dyn color=\"#%s\" bg=\"#%s\" material=\"%s\">%s</>"),
		*HexColor,
		*BgHex,
		*CurrentMatInstancePath,
		*WordText
	);

	CurrentTextWidget->AddTextWithParams(WordText, Color, CurrentSentenceBgColor, "", CurrentmaterialParameters);
	PreviousWord = WordText;
}

void URatingPaperWidget::HandleWordProcessed(FString word, FLinearColor const color)
{
	if (word == "\n" || word == "\\w")
	{
		CurrentTextWidget->AddText("\n");
		PreviousWord = "\n";
		return;
	}

	AddWordToPage(word, color);

}

void URatingPaperWidget::HandleScoreAdded(int32 ScoreToAdd, FString StringResponsible, FString Reason) {
	CheckWordTheme(StringResponsible, ScoreToAdd);
}

void URatingPaperWidget::HandleSentenceProcessed(TArray<ESentenceType> sentence_types, ESentenceStructureType sentence_struct) {
	if (sentence_types.IsEmpty()) {
		return;
	}

	SentenceStructCountMap.FindOrAdd(sentence_struct)++;
	RefreshSentenceStructCount(sentence_struct);

	// Handle BG color
	int NumColors = sentence_types.Num();
	CurrentSentenceBgColor = FLinearColor{ 0,0,0,0.f };
	for (int i = 0; i < NumColors; ++i) {
		const FLinearColor* NextColor = SentenceBgColor.Find(sentence_types[i]);
		SentenceTypesCountMap.FindOrAdd(sentence_types[i])++;
		if (!NextColor) continue;
		float T = 1.0f / (i + 1);
		CurrentSentenceBgColor = FLinearColor::LerpUsingHSV(CurrentSentenceBgColor, *NextColor, T);
	}

	RefreshSentenceTypesCount(sentence_types);
	CurrentmaterialParameters = *SentenceMaterialParameter.Find(sentence_struct);

	if (!IsDesignTime() && CurrentTextWidget) {
		CurrentTextWidget->GetTextBlock()->SetDefaultFont(FontInfo);
	}
}


void URatingPaperWidget::ScorePaperText(const FString& UserText)
{
	UWrittingReviewGameInstance* GI = Cast<UWrittingReviewGameInstance>(GetGameInstance());
	if (GI && GI->TextScoringSystem)
	{
		GI->TextScoringSystem->StartScoringWithDelay(UserText, this);
	}
}

void URatingPaperWidget::FindThemeItems() {
	TArray<AActor*> ThemeHolders;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AThemeHolder::StaticClass(), ThemeHolders);

	for (AActor* Actor : ThemeHolders)
	{
		AThemeHolder* Holder = Cast<AThemeHolder>(Actor);
		if (!Holder) continue;

		// Find attached components of type UItemTheme (if it’s a component)
		TArray<AActor*> Attached;
		Holder->GetAttachedActors(Attached);
		for (AActor* Child : Attached)
		{
			if (AItemTheme* ItemTheme = Cast<AItemTheme>(Child))
			{
				CurrentItemThemeActors.Add(ItemTheme);
			}
		}
	}
}

void URatingPaperWidget::CheckWordTheme(FString const& word, int32 freq_score) {

	for (auto item_theme : CurrentItemThemeActors) {
		if (auto found_weight = item_theme->GetThemeRelatedList().Find(word)) {
			HandleThemeFound(item_theme, word, freq_score, *found_weight);
		}
	}
}
