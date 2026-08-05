// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OperatorTypes.generated.h"

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
	TSoftObjectPtr<UTexture2D> Skin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Info;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FootModeMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D FootModeOffset;

	static TArray<FOperatorData> MakeFromDataRow(const FName& RowName, const FOperatorDataRow& Row)
	{
		TArray<FOperatorData> Result;
		for (const auto& Img : Row.SkinTextures)
		{
			Result.Add(FOperatorData{RowName, Img.Texture, Row.Info,Img.FootModeMultiplier, Img.FootModeOffset});
		}
		
		return Result;
	}
};