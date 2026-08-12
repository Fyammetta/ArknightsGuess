// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "GuessGameSettings.generated.h"

/**
 * Gameplay rule configuration and mode-to-component registry.
 * Edit in Project Settings → Game → Guess Game.
 *
 * ModeComponents maps a game-mode FGameplayTag to the ActorComponent subclass
 * that drives mode-specific logic on AGuessGameStateBase.
 * Every class in the map MUST implement IUGuessComponentInterface.
 */
UCLASS(config=Game, DefaultConfig, DisplayName="Guess Game")
class ARKNIGHTSGUESS_API UGuessGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UGuessGameSettings* Get()
	{
		return GetMutableDefault<UGuessGameSettings>();
	}

	// ---- Gameplay Rules ----
	UPROPERTY(config, EditAnywhere, Category = "Rules", meta = (ClampMin = "0"))
	int32 DefaultLevel = 36;

	UPROPERTY(config, EditAnywhere, Category = "Rules", meta = (ClampMin = "0"))
	int32 MaxGuessCount = 10;

	UPROPERTY(config, EditAnywhere, Category = "Rules", meta = (ClampMin = "0"))
	int32 ShuffleLimit = 20;

	UPROPERTY(config, EditAnywhere, Category = "Rules", meta = (ClampMin = "0"))
	int32 HintFrequency = 2;

	UPROPERTY(config, EditAnywhere, Category = "Rules", meta = (ClampMin = "1"))
	int32 ClarityPerLevel = 4;

	// ---- Mode → Component registry ----
	UPROPERTY(config, EditAnywhere, Category = "Mode")
	TMap<FGameplayTag, TSubclassOf<UActorComponent>> ModeComponents;

	// ---- Mode → Level mapping ----
	UPROPERTY(config, EditAnywhere, Category = "Mode", meta = (AllowedClasses = "/Script/Engine.World"))
	TMap<FGameplayTag, TSoftObjectPtr<UWorld>> ModeLevels;
};
