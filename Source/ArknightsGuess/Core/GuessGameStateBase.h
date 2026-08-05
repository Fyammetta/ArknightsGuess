// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "GuessGameStateBase.generated.h"

UCLASS()
class ARKNIGHTSGUESS_API AGuessGameStateBase : public AGameState
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = "OnRep_OnGuessStateChanged", BlueprintReadOnly)
	EGuessRoundState RoundState = EGuessRoundState::WaitingForPlayers;

	UPROPERTY(ReplicatedUsing = "OnRep_NextLevel", BlueprintReadOnly)
	int32 GuessCount = 0;

	UPROPERTY(ReplicatedUsing = "OnRep_NextRound", BlueprintReadOnly)
	int32 RoundNumber = 0;
	
	UPROPERTY(Replicated)
	FTriedAnswerArray TriedAnswers;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	EGuessRoundState GetGuessRoundState() const;

	UFUNCTION(BlueprintCallable)
	void SetGuessRoundState(EGuessRoundState State);

	UFUNCTION(BlueprintCallable)
	int32 GetGuessCount() const;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetupOperator(const FOperatorImage& Tex, const TArray<FString>& Hints);
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DisplayNextHint();
	
	UFUNCTION()
	void OnRep_OnGuessStateChanged();
	
	UFUNCTION()
	void OnRep_NextLevel();

	UFUNCTION()
	void OnRep_NextRound();

	void EnterNewRound(const FOperatorData& Operator);
	void Clarify();
};
