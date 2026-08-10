// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameModeBase.h"
#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess.h"


void AGuessGameModeBase::StartGame(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] StartGame | Mode=%s"), *Mode.ToString());
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->NetMulticast_StartGame(Mode);
	}
}

void AGuessGameModeBase::EndGame()
{
	UE_LOG(LogArknights, Log, TEXT("[GM] EndGame"));
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->NetMulticast_EndGame();
	}

	SetRoundState(EGuessRoundState::WaitingForPlayers);
}

void AGuessGameModeBase::StartNewRound()
{
	UE_LOG(LogArknights, Log, TEXT("[GM] StartNewRound"));
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!GS || !Subsystem)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] StartNewRound failed: no GameState or Subsystem"));
		return;
	}

	// Guard: don't start a new round if one is already in progress
	// (e.g. multiple clients calling RequestNextRound simultaneously)
	if (GS->GetGuessRoundState() == EGuessRoundState::Guessing)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] StartNewRound ignored: round already in progress"));
		return;
	}

	// 1. Pick an answer
	CorrectAnswer = Subsystem->GetRandomOperatorData();

	// 2. Push operator data to GameState
	GS->EnterNewRound(CorrectAnswer);
	

	// 3. Transition to Guessing
	SetRoundState(EGuessRoundState::Guessing);
}

void AGuessGameModeBase::ProcessGuess(const FName& OperatorName)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] ProcessGuess | Answer=%s"), *OperatorName.ToString());
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);

	if (!Subsystem || !GS || GS->GetGuessRoundState() != EGuessRoundState::Guessing)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] ProcessGuess failed: bad state or missing Subsystem/GS"));
		return;
	}

	if (OperatorName == CorrectAnswer.Name.RealName)
	{
		SetRoundState(EGuessRoundState::Verify);
	}
	else
	{	
		// else: wrong guess — client gets feedback, clarity level increases on GameState
		int32 Max = Subsystem->GetMaxGuessCount();
		int32 Cur = GS->GetGuessCount();
		if (Cur >= Max)
		{
			SetRoundState(EGuessRoundState::Reveal);
		}
		else
		{
			GS->Clarify();
		}
	}
}

void AGuessGameModeBase::SetRoundState(EGuessRoundState NewState) const
{
	UE_LOG(LogArknights, Log, TEXT("[GM] SetRoundState | State=%d"), static_cast<int32>(NewState));
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->SetGuessRoundState(NewState);
	}
}
