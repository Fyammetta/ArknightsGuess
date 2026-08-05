// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "OperatorTypes.generated.h"

UENUM(BlueprintType)
enum class EGuessRoundState : uint8
{
	WaitingForPlayers,  // Lobby
	Guessing,           // Clients can submit guesses
	Verify,             // Correct guess — show success
	Reveal,             // Out of guesses — show the answer
};

USTRUCT(BlueprintType)
struct FOperatorImage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 FootModeMultiplier = 8;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D FootModeOffset;
};

USTRUCT(BlueprintType)
struct FOperatorDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOperatorImage> SkinTextures;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TMap<FName, FString> Info;
};

USTRUCT(BlueprintType)
struct FOperatorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOperatorImage Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Info;


	static TArray<FOperatorData> MakeFromDataRow(const FName& RowName, const FOperatorDataRow& Row);
};

// ---- FastArray: tried-answer list replicated to clients ----
USTRUCT()
struct FTriedAnswerEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FName OperatorName;
};

USTRUCT()
struct FTriedAnswerArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTriedAnswerEntry> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FTriedAnswerEntry, FTriedAnswerArray>(Items, DeltaParams, *this);
	}

	FName GetLast() const
	{
		return Items.Num() > 0 ? Items.Last().OperatorName : NAME_None;
	}

	void Add(const FName& Name)
	{
		FTriedAnswerEntry& Entry = Items.AddDefaulted_GetRef();
		Entry.OperatorName = Name;
		MarkItemDirty(Entry);
	}

	void Clear()
	{
		Items.Empty();
		MarkArrayDirty();
	}
};

template<>
struct TStructOpsTypeTraits<FTriedAnswerArray> : public TStructOpsTypeTraitsBase2<FTriedAnswerArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
