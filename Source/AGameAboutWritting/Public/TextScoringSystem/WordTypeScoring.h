// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


static const TMap<FString, FString> POSTagMap = {
	{ TEXT("CC"), TEXT("coordinating conjunction") },
	{ TEXT("CD"), TEXT("cardinal digit") },
	{ TEXT("DT"), TEXT("determiner") },
	{ TEXT("EX"), TEXT("existential there") },
	{ TEXT("FW"), TEXT("foreign word") },
	{ TEXT("IN"), TEXT("preposition / subordinating conjunction") },
	{ TEXT("JJ"), TEXT("adjective (large)") },
	{ TEXT("JJR"), TEXT("adjective, comparative (larger)") },
	{ TEXT("JJS"), TEXT("adjective, superlative (largest)") },
	{ TEXT("LS"), TEXT("list item marker") },
	{ TEXT("MD"), TEXT("modal (could, will)") },
	{ TEXT("NN"), TEXT("noun, singular") },
	{ TEXT("NNS"), TEXT("noun plural") },
	{ TEXT("NNP"), TEXT("proper noun, singular") },
	{ TEXT("NNPS"), TEXT("proper noun, plural") },
	{ TEXT("PDT"), TEXT("predeterminer") },
	{ TEXT("POS"), TEXT("possessive ending (parent's)") },
	{ TEXT("PRP"), TEXT("personal pronoun (hers, herself, him, himself)") },
	{ TEXT("PRP$"), TEXT("possessive pronoun (her, his, mine, my, our)") },
	{ TEXT("RB"), TEXT("adverb (occasionally, swiftly)") },
	{ TEXT("RBR"), TEXT("adverb, comparative (greater)") },
	{ TEXT("RBS"), TEXT("adverb, superlative (biggest)") },
	{ TEXT("RP"), TEXT("particle (about)") },
	{ TEXT("SYM"), TEXT("symbol") },
	{ TEXT("TO"), TEXT("infinitive marker (to)") },
	{ TEXT("UH"), TEXT("interjection (goodbye)") },
	{ TEXT("VB"), TEXT("verb (ask)") },
	{ TEXT("VBG"), TEXT("verb, gerund (judging)") },
	{ TEXT("VBD"), TEXT("verb, past tense (pleaded)") },
	{ TEXT("VBN"), TEXT("verb, past participle (reunified)") },
	{ TEXT("VBP"), TEXT("verb, present tense (not 3rd person singular, wrap)") },
	{ TEXT("VBZ"), TEXT("verb, present tense (3rd person singular, bases)") },
	{ TEXT("WDT"), TEXT("wh-determiner (that, what)") },
	{ TEXT("WP"), TEXT("wh-pronoun (who)") },
	{ TEXT("WP$"), TEXT("possessive wh-pronoun") },
	{ TEXT("WRB"), TEXT("wh-adverb (how)") },
};

/**
 * 
 */
class UDataTable;

class AGAMEABOUTWRITTING_API WordTypeScoring
{
public:
	WordTypeScoring(UDataTable* word_type_datatable);

	FString GetWordType(FString const& word) const;

private:
	TMap<FString, FString> WordTypeMap;
};
