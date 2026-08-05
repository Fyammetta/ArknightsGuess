// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "Net/UnrealNetwork.h"

void AGuessGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGuessGameStateBase, RoundState);
	DOREPLIFETIME(AGuessGameStateBase, RoundNumber);
	DOREPLIFETIME(AGuessGameStateBase, TriedAnswers);
}

EGuessRoundState AGuessGameStateBase::GetGuessRoundState() const
{
	return RoundState;
}

void AGuessGameStateBase::SetGuessRoundState(EGuessRoundState State)
{
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

void AGuessGameStateBase::NetMulticast_SetupOperator_Implementation(const FOperatorImage& Tex, const TArray<FString>& Hints)
{
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
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.Broadcast(RoundState);
	}
}

void AGuessGameStateBase::OnRep_NextLevel()
{
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		if (Subsystem && GuessCount % 4 != 0)
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
	if (!HasAuthority()) return;

	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) return;

	RoundNumber++;
	GuessCount = 0;

	TArray<FString> Hints;
	Operator.Info.GenerateValueArray(Hints);
	
	for (int32 i = Hints.Num() - 1; i > 0; i--) {
		int32 j = FMath::Floor(FMath::Rand() * (i + 1)) % Hints.Num();
		auto Temp = Hints[i];
		Hints[i] = Hints[j];
		Hints[j] = Temp;
	}
	
	NetMulticast_SetupOperator(Operator.Image, Hints);
}

void AGuessGameStateBase::Clarify()
{
	if (!HasAuthority()) return;
	GuessCount++;
}
