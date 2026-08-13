// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameModeBase.h"

#include "ArknightsGuess.h"
#include "Core/DefaultGameStateBase.h"

void ADefaultGameModeBase::StartGame(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] StartGame | Mode=%s"), *Mode.ToString());
	if (ADefaultGameStateBase* GS = GetGameState<ADefaultGameStateBase>())
	{
		GS->NetMulticast_StartGame(Mode);
	}
	else
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] StartGame failed: no GameState"));
	}
}
