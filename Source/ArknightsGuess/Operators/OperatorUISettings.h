// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "OperatorTypes.h"
#include "OperatorUISettings.generated.h"

/**
 * Project-wide configuration for the operator guessing game.
 * Edit in Project Settings → Game → Operator UI.
 */
UCLASS(config=Game, DefaultConfig, DisplayName="Operator UI")
class ARKNIGHTSGUESS_API UOperatorUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UOperatorUISettings* Get()
	{
		return GetMutableDefault<UOperatorUISettings>();
	}

	// ---- Datas ----
	UPROPERTY(config, EditAnywhere, Category = "Datas")
	TMap<FGameplayTag, TSoftObjectPtr<UMaterial>> Materials;

	UPROPERTY(config, EditAnywhere, Category = "Datas")
	TSoftObjectPtr<UDataTable> OperatorDatas;

	// ---- Operator Sample ----
	UPROPERTY(config, EditAnywhere, Category = "Operator Sample")
	TSoftObjectPtr<UTexture2D> SampleTex;

	// ---- Gameplay Rules ----
	UPROPERTY(config, EditAnywhere, Category = "Gameplay Rules", meta = (ClampMin = "0"))
	int32 DefaultLevel = 36;

	UPROPERTY(config, EditAnywhere, Category = "Gameplay Rules", meta = (ClampMin = "0"))
	int32 MaxGuessCount = 10;

	UPROPERTY(config, EditAnywhere, Category = "Gameplay Rules", meta = (ClampMin = "0"))
	int32 ShuffleLimit = 20;

	UPROPERTY(config, EditAnywhere, Category = "Gameplay Rules", meta = (ClampMin = "0"))
	int32 HintFrequency = 2;
	
	UFUNCTION(BlueprintCallable, Category = "Datas")
	UMaterial* GetMaterial(const FGameplayTag& Mode);
};
