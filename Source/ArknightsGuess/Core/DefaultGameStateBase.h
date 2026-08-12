// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameState.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "DefaultGameStateBase.generated.h"

class IUGuessComponentInterface;

/**
 * Data layer — owns all replicated game-state properties, their accessors,
 * and the mode-specific gameplay component.
 *
 * AGuessGameStateBase inherits from this and adds RPC / flow-control logic.
 */
UCLASS()
class ARKNIGHTSGUESS_API ADefaultGameStateBase : public AGameState
{
	GENERATED_BODY()

public:
	ADefaultGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Game lifecycle RPCs ----
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_StartGame(const FGameplayTag& Mode);

	// ---- Round state ----
	UFUNCTION(BlueprintCallable)
	EGuessRoundState GetGuessRoundState() const;

	UFUNCTION(BlueprintCallable)
	void SetGuessRoundState(EGuessRoundState State);

	// ---- Guess counter ----
	UFUNCTION(BlueprintCallable)
	int32 GetGuessCount() const;

	// ---- Round number ----
	UFUNCTION(BlueprintCallable)
	int32 GetRoundNumber() const { return RoundNumber; }

	// ---- Mode component (created in BeginPlay from GuessGameSettings) ----
	TWeakInterfacePtr<IUGuessComponentInterface> GuessComponent;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = "OnRep_OnGuessStateChanged", BlueprintReadOnly)
	EGuessRoundState RoundState = EGuessRoundState::WaitingForPlayers;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 GuessCount = 0;

	UPROPERTY(ReplicatedUsing = "OnRep_NextRound", BlueprintReadOnly)
	int32 RoundNumber = 0;

	UPROPERTY(Replicated)
	FTriedAnswerArray TriedAnswers;

	UFUNCTION()
	void OnRep_OnGuessStateChanged();

	UFUNCTION()
	void OnRep_NextRound();

private:
	void GenerateGameplayComponent();
};