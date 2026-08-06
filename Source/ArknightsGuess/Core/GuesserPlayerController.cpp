// Fill out your copyright notice in the Description page of Project Settings.


#include "GuesserPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


void AGuesserPlayerController::StartGame_Implementation(const FName& Mode)
{
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->StartUp(Mode);
	}
}

void AGuesserPlayerController::EndGame_Implementation()
{
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->EndGame();
	}
}

void AGuesserPlayerController::ConfirmAnswer_Implementation(const FName& Answer)
{
	auto GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AGuessGameModeBase>() : nullptr;
	if (!GameMode) return;
	
	GameMode->ProcessGuess(Answer);
}

void AGuesserPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this);
	SetShowMouseCursor(true);
}
