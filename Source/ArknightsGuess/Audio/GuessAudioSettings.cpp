// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessAudioSettings.h"
#include "ArknightsGuess.h"

#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

USoundMix* UGuessAudioSettings::GetDefaultSoundMix() const
{
	return DefaultSoundMix.LoadSynchronous();
}

USoundClass* UGuessAudioSettings::GetSoundClassByTag(const FGameplayTag& Tag) const
{
	if (const auto* Found = SoundClassMapping.Find(Tag))
	{
		USoundClass* SoundClass = Found->LoadSynchronous();
		if (!SoundClass)
		{
			UE_LOG(LogArknights, Warning, TEXT("[GuessAudioSettings] Tag=%s mapped but asset failed to load: %s"), *Tag.ToString(), *Found->ToString());
		}
		return SoundClass;
	}

	UE_LOG(LogArknights, Warning, TEXT("[GuessAudioSettings] Tag=%s not found in SoundClassMapping (%d entries)"), *Tag.ToString(), SoundClassMapping.Num());
	return nullptr;
}
