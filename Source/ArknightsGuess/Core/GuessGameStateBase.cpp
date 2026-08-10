// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorUISettings.h"
#include "Net/UnrealNetwork.h"
#include "ArknightsGuess.h"

void AGuessGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGuessGameStateBase, RoundState);
	DOREPLIFETIME(AGuessGameStateBase, GuessCount);
	DOREPLIFETIME(AGuessGameStateBase, RoundNumber);
	DOREPLIFETIME(AGuessGameStateBase, TriedAnswers);
}

EGuessRoundState AGuessGameStateBase::GetGuessRoundState() const
{
	return RoundState;
}

void AGuessGameStateBase::SetGuessRoundState(EGuessRoundState State)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] SetGuessRoundState | State=%d | Authority=%d"), static_cast<int32>(State), HasAuthority());
	RoundState = State;

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.Broadcast(State);
	}
}

int32 AGuessGameStateBase::GetGuessCount() const
{
	return GuessCount;
}

void AGuessGameStateBase::NetMulticast_StartGame_Implementation(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] NetMulticast_StartGame | Mode=%s | Authority=%d"), *Mode.ToString(), HasAuthority());
	if (HasAuthority()) return;

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->StartUp(Mode);
	}
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
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnNextHintsDisplayAllowed.Broadcast();
	}
}

void AGuessGameStateBase::OnRep_OnGuessStateChanged()
{
	UE_LOG(LogArknights, Log, TEXT("[GS] OnRep_GuessStateChanged | State=%d"), static_cast<int32>(RoundState));
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.Broadcast(RoundState);
	}
}

void AGuessGameStateBase::OnRep_NextLevel()
{
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		if (Subsystem && GuessCount % UOperatorUISettings::Get()->ClarityPerLevel != 0)
		{
			Subsystem->OnGuessProcessChanged.Broadcast(RoundNumber, Subsystem->GetDefaultLevel() - GuessCount);
		}
	}
}

void AGuessGameStateBase::OnRep_NextRound()
{
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessProcessChanged.Broadcast(RoundNumber, Subsystem->GetDefaultLevel());
	}
}

void AGuessGameStateBase::EnterNewRound(const FOperatorData& Operator)
{
	UE_LOG(LogArknights, Log, TEXT("[GS] EnterNewRound | Answer=%s | Authority=%d"), *Operator.Name.RealName.ToString(), HasAuthority());
	if (!HasAuthority()) return;

	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) return;

	RoundNumber++;
	GuessCount = 0;

	TArray<FString> Hints;
	for (const TPair<FName, FString>& Info : Operator.Info)
	{
		Hints.Add(Info.Key.ToString() + "/" + Info.Value);
	}

	for (int32 i = Hints.Num() - 1; i > 0; i--) {
		int32 j = FMath::Floor(FMath::Rand() * (i + 1)) % Hints.Num();
		auto Temp = Hints[i];
		Hints[i] = Hints[j];
		Hints[j] = Temp;
	}

	NetMulticast_SetupOperator(Operator.Image, Hints);

	// Direct broadcast for single-player: OnRep_NextRound won't fire without replication
	Subsystem->OnGuessProcessChanged.Broadcast(RoundNumber, Subsystem->GetDefaultLevel());
}

void AGuessGameStateBase::Clarify()
{
	UE_LOG(LogArknights, Log, TEXT("[GS] Clarify | GuessCount=%d | Authority=%d"), GuessCount, HasAuthority());
	if (!HasAuthority()) return;
	GuessCount++;

	// Direct broadcast for single-player: OnRep_NextLevel won't fire without replication
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		if (GuessCount % UOperatorUISettings::Get()->ClarityPerLevel != 0)
		{
			Subsystem->OnGuessProcessChanged.Broadcast(RoundNumber, Subsystem->GetDefaultLevel() - GuessCount);
		}

		// Show next hint every HintFrequency wrong guesses
		if (GuessCount > 0 && GuessCount % Subsystem->GetHintFrequency() == 0)
		{
			NetMulticast_DisplayNextHint();
		}
	}
}
