// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessAudioSubsystem.h"
#include "ArknightsGuess.h"
#include "Audio/GuessAudioSettings.h"
#include "Core/GuessGamerSettings.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Operators/OperatorTags.h"
#include "Operators/OperatorTypes.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundWave.h"

UGuessAudioSubsystem* UGuessAudioSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !WorldContextObject->GetWorld() || !WorldContextObject->GetWorld()->GetGameInstance()) return nullptr;
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UGuessAudioSubsystem>();
}

void UGuessAudioSubsystem::ApplySavedVolumes()
{
	auto* Settings = UGuessGamerSettings::Get();
	const FGameplayTag Tags[] = { SoundTags::Default(), SoundTags::Music(), SoundTags::UI(), SoundTags::Voice() };
	for (const FGameplayTag& Tag : Tags)
	{
		ApplyVolume(Tag, Settings ? Settings->GetVolumeByTag(Tag) : 1.0f);
	}
}

void UGuessAudioSubsystem::ApplyVolume(const FGameplayTag& Tag, float Volume)
{
	auto* AudioSettings = UGuessAudioSettings::Get();
	USoundMix* Mix = AudioSettings ? AudioSettings->GetDefaultSoundMix() : nullptr;
	if (!Mix)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GuessAudio] ApplyVolume | Tag=%s Volume=%.2f aborted: no sound mix"), *Tag.ToString(), Volume);
		return;
	}

	if (USoundClass* SoundClass = AudioSettings->GetSoundClassByTag(Tag))
	{
		UGameplayStatics::SetSoundMixClassOverride(GetGameInstance(), Mix, SoundClass, Volume, 1.0f, 0.0f);
		UE_LOG(LogArknights, Log, TEXT("[GuessAudio] ApplyVolume | Tag=%s Class=%s Volume=%.2f"), *Tag.ToString(), *GetNameSafe(SoundClass), Volume);
	}
}

// ---- Content tables ----

void UGuessAudioSubsystem::LoadTables()
{
	if (OperatorVoiceTable || BackgroundMusicTable) return;

	auto* AudioSettings = UGuessAudioSettings::Get();
	if (!AudioSettings) return;

	OperatorVoiceTable = AudioSettings->OperatorVoiceTable.LoadSynchronous();
	BackgroundMusicTable = AudioSettings->BackgroundMusicTable.LoadSynchronous();
}

UDataTable* UGuessAudioSubsystem::GetOperatorVoiceTable()
{
	LoadTables();
	return OperatorVoiceTable;
}

UDataTable* UGuessAudioSubsystem::GetBackgroundMusicTable()
{
	LoadTables();
	return BackgroundMusicTable;
}

USoundWave* UGuessAudioSubsystem::GetOperatorVoiceSuccess(const FName& OperatorName)
{
	LoadTables();
	if (!OperatorVoiceTable) return nullptr;

	const FOperatorVoiceRow* Row = OperatorVoiceTable->FindRow<FOperatorVoiceRow>(OperatorName, TEXT("[GuessAudioSubsystem] Voice Success"));
	USoundWave* Voice = Row ? Row->Success.LoadSynchronous() : nullptr;

	return Voice;
}

USoundWave* UGuessAudioSubsystem::GetOperatorVoiceFailure(const FName& OperatorName)
{
	LoadTables();
	if (!OperatorVoiceTable) return nullptr;

	const FOperatorVoiceRow* Row = OperatorVoiceTable->FindRow<FOperatorVoiceRow>(OperatorName, TEXT("[GuessAudioSubsystem] Voice Failure"));
	USoundWave* Voice = Row ? Row->Failure.LoadSynchronous() : nullptr;

	return Voice;
}

const FBackgroundMusicRow* UGuessAudioSubsystem::GetBackgroundMusic(const FName& MusicName)
{
	LoadTables();
	return BackgroundMusicTable ? BackgroundMusicTable->FindRow<FBackgroundMusicRow>(MusicName, TEXT("[GuessAudioSubsystem] Music")) : nullptr;
}

USoundWave* UGuessAudioSubsystem::GetBackgroundMusicSource(const FName& MusicName)
{
	USoundWave* Source = nullptr;
	if (const FBackgroundMusicRow* Row = GetBackgroundMusic(MusicName))
	{
		Source = Row->Source.LoadSynchronous();
	}

	return Source;
}
