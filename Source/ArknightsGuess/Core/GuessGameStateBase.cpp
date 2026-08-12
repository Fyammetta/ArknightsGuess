// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "GuessGame/UGuessComponentInterface.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess.h"
#include "GameFramework/PlayerState.h"

// ---- Player tracking ----

void AGuessGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	UE_LOG(LogArknights, Log, TEXT("[GS] AddPlayerState | Player=%s"), PlayerState ? *PlayerState->GetPlayerName() : TEXT("null"));
	if (PlayerState)
		NetMulticast_BroadcastPlayerCountChanged(PlayerState->GetPlayerController(), Join);
}

void AGuessGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	UE_LOG(LogArknights, Log, TEXT("[GS] RemovePlayerState | Player=%s"), PlayerState ? *PlayerState->GetPlayerName() : TEXT("null"));
	if (PlayerState)
		NetMulticast_BroadcastPlayerCountChanged(PlayerState->GetPlayerController(), Leave);
}

void AGuessGameStateBase::NetMulticast_BroadcastPlayerCountChanged_Implementation(APlayerController* Player, EPlayerChangeType Type)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] NetMulticast_BroadcastPlayerCountChanged | Type=%d | Authority=%d"), static_cast<int32>(Type), HasAuthority());
	if (Player)
		OnPlayerCountChanged.Broadcast(Player, Type);
}



void AGuessGameStateBase::NetMulticast_EndGame_Implementation()
{
	UE_LOG(LogArknights, Log, TEXT("[GS] NetMulticast_EndGame | Authority=%d"), HasAuthority());
	if (HasAuthority()) return;

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->EndGame();
	}
}

void AGuessGameStateBase::NetMulticast_SetupOperator_Implementation(const FOperatorImage& Tex, const TArray<FString>& Hints)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] NetMulticast_SetupOperator | Hints=%d | Authority=%d"), Hints.Num(), HasAuthority());
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnOperatorDataReceived.Broadcast(Tex, Hints);
	}
}

void AGuessGameStateBase::NetMulticast_DisplayNextHint_Implementation()
{
	UE_LOG(LogArknights, Log, TEXT("[GS] NetMulticast_DisplayNextHint | Authority=%d"), HasAuthority());
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnNextHintsDisplayAllowed.Broadcast();
	}
}

// ---- Round orchestration ----

void AGuessGameStateBase::NetMulticast_BroadcastOnPlayerReady_Implementation(APlayerController* Player, bool bReady, const FGameplayTag& Message)
{
	WhenPlayerOnReady.Broadcast(Player, bReady, Message);
}

void AGuessGameStateBase::EnterNewRound(const FOperatorData& Operator)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] EnterNewRound | Answer=%s | Authority=%d"), *Operator.Name.RealName.ToString(), HasAuthority());
	if (!HasAuthority()) return;

	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) return;

	RoundNumber++;
	GuessCount = 0;

	if (GuessComponent.IsValid())
		GuessComponent->OnNewRound();
	else
		UE_LOG(LogArknights, Warning, TEXT("[GS] EnterNewRound: GuessComponent is invalid"));

	TArray<FString> Hints;
	for (const TPair<FName, FString>& Info : Operator.Info)
	{
		Hints.Add(Info.Key.ToString() + "/" + Info.Value);
	}

	for (int32 i = Hints.Num() - 1; i > 0; i--) {
		const int32 j = FMath::RandRange(0, i);
		Hints.Swap(i, j);
	}

	NetMulticast_SetupOperator(Operator.Image, Hints);
}

void AGuessGameStateBase::Clarify()
{
	UE_LOG(LogArknights, Log, TEXT("[GS] Clarify | GuessCount=%d | Authority=%d"), GuessCount, HasAuthority());
	if (!HasAuthority()) return;
	GuessCount++;

	if (GuessComponent.IsValid())
		GuessComponent->OnWrongGuess(GuessCount);
	else
		UE_LOG(LogArknights, Warning, TEXT("[GS] Clarify: GuessComponent is invalid"));

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		if (GuessCount > 0 && GuessCount % Subsystem->GetHintFrequency() == 0)
		{
			NetMulticast_DisplayNextHint();
		}
	}
}
