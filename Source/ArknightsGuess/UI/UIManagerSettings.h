// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "UIManagerSettings.generated.h"

/**
 * Maps GameplayTags to UUserWidget classes for the UIManagerSubsystem to lazy-load.
 * Edit in Project Settings → Game → UI Manager.
 */
UCLASS(config=Game, DefaultConfig, DisplayName="UI Manager")
class ARKNIGHTSGUESS_API UUIManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UUIManagerSettings* Get()
	{
		return GetDefault<UUIManagerSettings>();
	}

	/** FGameplayTag → Widget class. The subsystem creates on first ShowUI() call. */
	UPROPERTY(config, EditAnywhere, Category = "UI Registry", meta = (TitleProperty = "Tag"))
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> UIRegistry;

	/** Minimum time (seconds) the loading screen stays visible. */
	UPROPERTY(config, EditAnywhere, Category = "UI Registry")
	float MinLoadingTime = 0.5f;

	/** Z-order applied when AddToViewport (higher = on top). */
	UPROPERTY(config, EditAnywhere, Category = "UI Registry")
	int32 DefaultZOrder = 0;
};
