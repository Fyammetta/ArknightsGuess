// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultGameStateBase.h"
#include "GameplayTagContainer.h"
#include "GuessGameStateBase.generated.h"

UENUM()
enum EPlayerChangeType
{
	Join,
	Leave
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerCountChangedDelegate, APlayerController*, Player, EPlayerChangeType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPlayerOnReadyDelegate, APlayerController*, Player, bool , bReady, const FGameplayTag&, Message);


/**
 * RPC / flow-control layer — player tracking, game-lifecycle Multicast RPCs,
 * and round orchestration (EnterNewRound / Clarify).
 *
 * Data properties and accessors live in ADefaultGameStateBase.
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuessGameStateBase : public ADefaultGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPlayerCountChangedDelegate OnPlayerCountChanged;

	UPROPERTY(BlueprintAssignable)
	FPlayerOnReadyDelegate WhenPlayerOnReady;
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastPlayerCountChanged(APlayerController* Player, EPlayerChangeType Type);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_StartGame(const FGameplayTag& Mode);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_EndGame();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetupOperator(const FOperatorImage& Tex, const TArray<FString>& Hints);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_DisplayNextHint();
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastOnPlayerReady(APlayerController* Player, bool bReady, const FGameplayTag& Message);

	void EnterNewRound(const FOperatorData& Operator);
	void Clarify();
};
