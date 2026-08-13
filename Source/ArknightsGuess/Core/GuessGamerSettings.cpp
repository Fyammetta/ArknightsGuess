// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessGamerSettings.h"

UGuessGamerSettings* UGuessGamerSettings::Get()
{
	return Cast<UGuessGamerSettings>(GetGameUserSettings());
}

FString UGuessGamerSettings::GetPlayerName()
{
	if (!Get()) return "";
	
	return Get()->PlayerName;
}

UTexture2D* UGuessGamerSettings::GetPlayerIcon()
{
	if (!Get()) return nullptr;
	
	return Get()->PlayerIcon.LoadSynchronous();
}

float UGuessGamerSettings::GetVolumeByTag(const FGameplayTag& Tag)
{
	if (!Get() || !Get()->VolumeMapping.Contains(Tag)) return 1.0f;

	return Get()->VolumeMapping[Tag];
}

void UGuessGamerSettings::SetVolumeByTag(const FGameplayTag& Tag, float Volume)
{
	if (!Get()) return;
	
	Get()->VolumeMapping.FindOrAdd(Tag) = Volume;
}

void UGuessGamerSettings::SetPlayerName(const FString& Name)
{
	if (!Get()) return ;
	
	Get()->PlayerName = Name;
}

void UGuessGamerSettings::SetPlayerIcon(UTexture2D* Icon)
{
	if (!Get() || !Icon) return;
	Get()->PlayerIcon = Icon->GetPathName();
}
