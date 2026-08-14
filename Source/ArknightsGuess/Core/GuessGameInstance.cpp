// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessGameInstance.h"
#include "ArknightsGuess.h"
#include "Audio/GuessAudioSettings.h"
#include "Audio/GuessAudioSubsystem.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundMix.h"

void UGuessGameInstance::OnStart()
{
	Super::OnStart();

	SetupResolution();
	StartAudio();
}

void UGuessGameInstance::SetupResolution()
{
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	auto Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;
	if (Resolutions.Num() == 0) return;

	auto Resolution = Settings->GetScreenResolution();
	for (const auto& Info : Resolutions)
	{
		if (Resolution == Info)
		{
			return;
		}
	}
	
	Settings->SetScreenResolution(Resolutions.Last());
}

void UGuessGameInstance::StartAudio()
{
	auto* AudioSettings = UGuessAudioSettings::Get();
	USoundMix* Mix = AudioSettings ? AudioSettings->GetDefaultSoundMix() : nullptr;
	if (!Mix)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GuessGameInstance] StartAudio aborted: DefaultSoundMix is null"));
		return;
	}

	UGameplayStatics::PushSoundMixModifier(this, Mix);
	UE_LOG(LogArknights, Log, TEXT("[GuessGameInstance] StartAudio | Mix=%s"), *GetNameSafe(Mix));

	if (auto* AudioSub = GetSubsystem<UGuessAudioSubsystem>())
	{
		AudioSub->ApplySavedVolumes();
	}
}
