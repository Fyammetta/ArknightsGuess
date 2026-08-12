// Fill out your copyright notice in the Description page of Project Settings.

#include "GuesserPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GuessGame/GuessGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Operators/OperatorFunctionLibrary.h"
#include "Operators/OperatorSubsystem.h"
#include "UI/UIManagerSettings.h"
#include "UI/UIManagerSubsystem.h"

void AGuesserPlayerController::BeginPlay()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] BeginPlay"));
	Super::BeginPlay();

	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this);
	SetShowMouseCursor(true);

	// Default loading / HUD tags (blueprint can override)

	// Bind loading-flow delegates
	if (auto Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnOperatorDataReceived.AddDynamic(this, &AGuesserPlayerController::OnOperatorDataReady);
		Sub->OnGuessGameEnd.AddDynamic(this, &AGuesserPlayerController::OnGameEnd);
	}
	
	if (auto* UIMgr = UUIManagerSubsystem::Get(this))
	{
		UIMgr->ShowUI(InitialUITag);
	}
	
}

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

void AGuesserPlayerController::EndGame_Implementation()
{
	if (!IsLocalController())
	{
		UE_LOG(LogArknights, Log, TEXT("[PC] EndGame from client — returning to local main menu"));
		ClientTravel(TEXT("/Game/Maps/Map_MainLevel"), TRAVEL_Absolute, false);
		return;
	}

	UE_LOG(LogArknights, Log, TEXT("[PC] EndGame"));
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!Subsystem || !GM) { UE_LOG(LogArknights, Warning, TEXT("[PC] EndGame failed: no Subsystem or GameMode")); return; }
	
	Subsystem->EndGame();
	GM->EndGame();
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(this,UGuessGameSettings::Get()->ModeLevels[FGameplayTag()],true, TEXT(""));
}

void AGuesserPlayerController::OnGameEnd()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnGameEnd -> Show Loading"));
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[DefaultPC] OnGameEnd failed: no World")); return; }
}

void AGuesserPlayerController::OnOperatorDataReady(const FOperatorImage& Tex, const TArray<FString>& Hints)
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnOperatorDataReady | Hints=%d"), Hints.Num());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[DefaultPC] OnOperatorDataReady failed: no World")); return; }
}