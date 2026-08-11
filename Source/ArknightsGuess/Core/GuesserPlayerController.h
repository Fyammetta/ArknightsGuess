// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultPlayerController.h"
#include "GuesserPlayerController.generated.h"

/**
 * In-game PlayerController — inherits UI lifecycle and startup RPCs from ADefaultPlayerController.
 * Adds in-game RPCs: ConfirmAnswer / RequestNextRound.
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuesserPlayerController : public ADefaultPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void ConfirmAnswer(const FName& Answer);

	UFUNCTION(Reliable, Server, BlueprintCallable)
	void RequestNextRound();
};
