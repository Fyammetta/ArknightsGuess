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
	
	UPROPERTY()
	TSet<APlayerController*> ReadyPlayers;
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	

public:

	// ---- Round flow (server-only, called by PlayerController RPCs) ----
	void StartGame(const FGameplayTag& Mode);
	void EndGame();
	void TryStartNewRound(APlayerController* Player);
	void ProcessGuess(const FName& OperatorName);
	
	int32 GetReadyPlayerCount() const;

protected:
	void SetRoundState(EGuessRoundState NewState) const;
	void StartNewRound();

};
