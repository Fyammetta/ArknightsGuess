// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "GameFramework/GameMode.h"
#include "GuessGameModeBase.generated.h"

class AGuessGameStateBase;
class UOperatorSubsystem;

UCLASS()
class ARKNIGHTSGUESS_API AGuessGameModeBase : public AGameMode
{
	GENERATED_BODY()

	UPROPERTY()
	FOperatorData CorrectAnswer;
protected:
	

public:

	// ---- Round flow (server-only, called by PlayerController RPCs) ----
	void StartGame(const FGameplayTag& Mode);
	void EndGame();
	void StartNewRound();
	void ProcessGuess(const FName& OperatorName);

protected:
	void SetRoundState(EGuessRoundState NewState) const;
};
