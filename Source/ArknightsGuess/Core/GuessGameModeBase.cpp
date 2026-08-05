// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameModeBase.h"
#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Blueprint/UserWidget.h"

void AGuessGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	MainUIWidget = CreateWidget<UUserWidget>(GetWorld(), MainUIClass,TEXT("MainUIWidget"));
	MainUIWidget->AddToViewport();
}

void AGuessGameModeBase::StartNewRound()
{
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!GS || !Subsystem)
	{
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
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);

	if (!Subsystem || !GS || GS->GetGuessRoundState() != EGuessRoundState::Guessing)
	{
		return;
	}

	if (OperatorName == CorrectAnswer.Name)
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
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->SetGuessRoundState(NewState);
	}
}
