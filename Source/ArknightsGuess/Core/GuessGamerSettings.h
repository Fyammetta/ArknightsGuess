// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameUserSettings.h"
#include "GuessGamerSettings.generated.h"

/**
 *
 */
UCLASS()
class ARKNIGHTSGUESS_API UGuessGamerSettings : public UGameUserSettings
{
	GENERATED_BODY()
public:
	static UGuessGamerSettings* Get();

	static FString GetPlayerName();

	static UTexture2D* GetPlayerIcon();

	static float GetVolumeByTag(const FGameplayTag& Tag);

	static void SetPlayerName(const FString& Name);

	static void SetPlayerIcon(UTexture2D* Icon);

	static void SetVolumeByTag(const FGameplayTag& Tag, float Volume);

protected:
	UPROPERTY(BlueprintReadWrite, Config, Category = "Multiplay")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Config, Category = "Multiplay")
	TSoftObjectPtr<UTexture2D> PlayerIcon;

	UPROPERTY(BlueprintReadWrite, Config, Category = "Audio")
	TMap<FGameplayTag, float> VolumeMapping;
};
