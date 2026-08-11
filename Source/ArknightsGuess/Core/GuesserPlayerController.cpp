// Fill out your copyright notice in the Description page of Project Settings.

#include "GuesserPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess.h"

void AGuesserPlayerController::ConfirmAnswer_Implementation(const FName& Answer)
{
	UE_LOG(LogArknights, Log, TEXT("[GuesserPC] ConfirmAnswer | Answer=%s"), *Answer.ToString());
	auto GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AGuessGameModeBase>() : nullptr;
	if (!GameMode) { UE_LOG(LogArknights, Warning, TEXT("[GuesserPC] ConfirmAnswer failed: no GameMode")); return; }

	GameMode->ProcessGuess(Answer);
}

void AGuesserPlayerController::RequestNextRound_Implementation()
{
	UE_LOG(LogArknights, Log, TEXT("[GuesserPC] RequestNextRound"));
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!GM) { UE_LOG(LogArknights, Warning, TEXT("[GuesserPC] RequestNextRound failed: no GameMode")); return; }
	GM->TryStartNewRound(this);
}
