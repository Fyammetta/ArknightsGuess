// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultGameStateBase.h"
#include "GuessGameStateBase.generated.h"



/**
 * RPC / flow-control layer — player tracking, round orchestration (EnterNewRound / Clarify).
 *
 * Data properties and accessors live in ADefaultGameStateBase.
 */

DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerAnsweredDelegate, APlayerState*, const FName&);
UCLASS()
class ARKNIGHTSGUESS_API AGuessGameStateBase : public ADefaultGameStateBase
{
	GENERATED_BODY()

public:
	FPlayerAnsweredDelegate OnPlayerAnswered;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastPlayerCountChanged(APlayerController* Player, EPlayerChangeType Type);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetupOperator(const FOperatorImage& Tex, const TArray<FString>& Hints);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DisplayNextHint();
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_OnPlayerAnswered(APlayerState* Player,const  FName& Answer);
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_EndGame();

	void EnterNewRound(const FOperatorData& Operator);
	void Clarify();
};