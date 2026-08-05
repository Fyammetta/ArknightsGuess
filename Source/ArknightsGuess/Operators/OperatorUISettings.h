// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OperatorUISettings.generated.h"

/**
 *
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
	
	UPROPERTY(config, EditAnywhere)
	TMap<FName, TSoftObjectPtr<UMaterial>> Materials;

	UPROPERTY(config, EditAnywhere)
	TSoftObjectPtr<UTexture2D> SampleTex;
};
