// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void EndGame();
	void SetPlayerPrepared(APlayerController* Player);
	void SetPlayerUnprepared(APlayerController* Player);

	void ProcessGuess(APlayerController* Player,const FName& OperatorName);
	
	void ResetPreparedPlayers();

	int32 GetReadyPlayerCount() const;


protected:
	void SetRoundState(EGuessRoundState NewState) const;
	void StartNewRound();
	
private:

	
	UFUNCTION()
	void OnAllReadyForNextRound(APlayerState* PC, bool Ready);

};