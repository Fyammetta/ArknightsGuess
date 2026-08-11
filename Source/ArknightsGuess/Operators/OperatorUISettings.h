// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "OperatorTypes.h"
#include "OperatorUISettings.generated.h"

/**
 * UI-facing configuration: materials, textures, and data table references.
 * Gameplay rules have moved to UGuessGameSettings (Project Settings → Game → Guess Game).
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

	UFUNCTION(BlueprintCallable, Category = "Datas")
	UMaterial* GetMaterial(const FGameplayTag& Mode);
};
