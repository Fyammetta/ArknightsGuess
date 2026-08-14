// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GuessAudioSubsystem.generated.h"

struct FBackgroundMusicRow;
class UDataTable;
class USoundWave;

/**
 *
 */
UCLASS()
class ARKNIGHTSGUESS_API UGuessAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	
	static UGuessAudioSubsystem* Get(const UObject* WorldContextObject);

	/** Apply saved volumes for all channels. */
	void ApplySavedVolumes();

	/** Apply a volume override for the given channel immediately. */
	void ApplyVolume(const FGameplayTag& Tag, float Volume);

	// ---- Content tables (lazy-loaded, strong-referenced) ----
	UDataTable* GetOperatorVoiceTable();

	UDataTable* GetBackgroundMusicTable();

	USoundWave* GetOperatorVoiceSuccess(const FName& OperatorName);

	USoundWave* GetOperatorVoiceFailure(const FName& OperatorName);

	const FBackgroundMusicRow* GetBackgroundMusic(const FName& MusicName);

	USoundWave* GetBackgroundMusicSource(const FName& MusicName);

protected:
	UPROPERTY()
	UDataTable* OperatorVoiceTable;

	UPROPERTY()
	UDataTable* BackgroundMusicTable;

private:
	void LoadTables();
};
