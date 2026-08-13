// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GuessAudioSettings.generated.h"

class UDataTable;
class USoundClass;
class USoundMix;

/**
 * Audio assets configuration: sound mix and sound classes per channel.
 * Edit in Project Settings → Game → Guess Audio.
 */
UCLASS(config=Game, DefaultConfig, DisplayName="Guess Audio")
class ARKNIGHTSGUESS_API UGuessAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UGuessAudioSettings* Get()
	{
		return GetMutableDefault<UGuessAudioSettings>();
	}

	USoundMix* GetDefaultSoundMix() const;

	USoundClass* GetSoundClassByTag(const FGameplayTag& Tag) const;

	UPROPERTY(config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundMix> DefaultSoundMix;

	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (TitleProperty = "Tag"))
	TMap<FGameplayTag, TSoftObjectPtr<USoundClass>> SoundClassMapping;

	// ---- Audio content tables ----
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<UDataTable> OperatorVoiceTable;

	UPROPERTY(config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<UDataTable> BackgroundMusicTable;
};
