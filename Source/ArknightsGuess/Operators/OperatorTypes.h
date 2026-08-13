// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "OperatorTags.h"
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableNames;
};

USTRUCT(BlueprintType)
struct FOperatorNamePair
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RealName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SearchName;
	FOperatorNamePair() : RealName(NAME_None), SearchName(TEXT("")) {}
	
	FOperatorNamePair(const FName& Name, const TArray<FName>& AvailableNames);

	friend uint32 GetTypeHash(const FOperatorNamePair& Pair)
	{
		return GetTypeHash(Pair.RealName);
	}
	
	friend bool operator==(const FOperatorNamePair& A, const FOperatorNamePair& B)
	{
		return A.RealName == B.RealName;
	}
};

USTRUCT(BlueprintType)
struct FOperatorData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOperatorNamePair Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOperatorImage Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Info;
	
	static TArray<FOperatorData> MakeFromDataRow(const FName& RowName, const FOperatorDataRow& Row);
	
	FOperatorData() : Name(NAME_None,{}), Image(FOperatorImage()), Info({}){};
};

USTRUCT(BlueprintType)
struct FOperatorIconRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
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

USTRUCT(BlueprintType)
struct FOperatorVoiceRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Success;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Failure;
};

USTRUCT(BlueprintType)
struct FBackgroundMusicRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Author;
};

