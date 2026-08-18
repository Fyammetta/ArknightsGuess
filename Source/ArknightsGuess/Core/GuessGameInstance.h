// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GuessGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UGuessGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void OnStart() override;
	
	void SetupResolution();
	void StartAudio();
	
	void OnNetConnectFailed(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureReason, const FString& ErrorMessage);
	void OnTravelFailed(UWorld* World, ETravelFailure::Type FailureReason, const FString& ErrorMessage);
};
