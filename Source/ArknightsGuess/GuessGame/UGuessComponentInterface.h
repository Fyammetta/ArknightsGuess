// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UGuessComponentInterface.generated.h"

UINTERFACE()
class UUGuessComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Common interface for per-mode components owned by AGuessGameStateBase.
 * Each game mode (Mosaic, Part, etc.) gets its own component that implements
 * OnNewRound / OnWrongGuess with mode-specific logic.
 */
class ARKNIGHTSGUESS_API IUGuessComponentInterface
{
	GENERATED_BODY()

public:
	/** Called when a new round starts — initialize mode-specific state, broadcast clarity. */
	virtual void OnNewRound() = 0;

	/** Called after a wrong guess — advance mode-specific state, broadcast clarity. */
	virtual void OnWrongGuess(int32 GuessCount) = 0;
};
