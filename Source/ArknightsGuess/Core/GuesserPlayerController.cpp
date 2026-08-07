// Fill out your copyright notice in the Description page of Project Settings.


#include "GuesserPlayerController.h"

#include "GuessGameModeBase.h"
#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/UI/UIManagerSettings.h"
#include "ArknightsGuess/UI/UIManagerSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ArknightsGuess.h"


void AGuesserPlayerController::StartGame_Implementation(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] StartGame | Mode=%s"), *Mode.ToString());
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!Subsystem || !GM) { UE_LOG(LogArknights, Warning, TEXT("[PC] StartGame failed: no Subsystem or GameMode")); return; }

	// 1. Multicast StartGame to ALL clients — initializes their Subsystem (OperatorNames, etc.)
	//    HasAuthority() guard in NetMulticast_StartGame prevents double-init on server.
	GM->StartGame(Mode);

	// 2. Start the server Subsystem — broadcasts OnGuessGameStart, which triggers
	//    PC::OnGameStart → show Loading → RequestNextRound → GM::StartNewRound()
	Subsystem->StartUp(Mode);

	// 3. Sync gameplay settings to the owning client's Subsystem
	NetMulticast_SyncGameSettings(
		Subsystem->GetDefaultLevel(),
		Subsystem->GetShuffleLimit(),
		Subsystem->GetMaxGuessCount(),
		Subsystem->GetHintFrequency());
}

void AGuesserPlayerController::EndGame_Implementation()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] EndGame"));
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!Subsystem || !GM) { UE_LOG(LogArknights, Warning, TEXT("[PC] EndGame failed: no Subsystem or GameMode")); return; }

	Subsystem->EndGame();
	GM->EndGame();
}

void AGuesserPlayerController::ConfirmAnswer_Implementation(const FName& Answer)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] ConfirmAnswer | Answer=%s"), *Answer.ToString());
	auto GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AGuessGameModeBase>() : nullptr;
	if (!GameMode) { UE_LOG(LogArknights, Warning, TEXT("[PC] ConfirmAnswer failed: no GameMode")); return; }

	GameMode->ProcessGuess(Answer);
}

void AGuesserPlayerController::BeginPlay()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] BeginPlay"));
	Super::BeginPlay();

	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this);
	SetShowMouseCursor(true);

	// Default loading / HUD tags (blueprint can override)
	if (!LoadingUITag.IsValid())
		LoadingUITag = FGameplayTag::RequestGameplayTag("Main.Loading");
	if (!GameHUDTag.IsValid())
		GameHUDTag = FGameplayTag::RequestGameplayTag("GameMode.Mosaic");

	// Bind loading-flow delegates
	if (auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnGuessGameStart.AddDynamic(this, &AGuesserPlayerController::OnGameStart);
		Sub->OnOperatorDataReceived.AddDynamic(this, &AGuesserPlayerController::OnOperatorDataReady);
		Sub->OnGuessGameEnd.AddDynamic(this, &AGuesserPlayerController::OnGameEnd);
	}

	if (InitialUITag.IsValid())
	{
		if (auto* UIMgr = UUIManagerSubsystem::Get(this))
		{
			UIMgr->ShowUI(InitialUITag);
		}
	}
}


void AGuesserPlayerController::OnGameStart()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] OnGameStart -> Show Loading"));
	LoadingStartTime = GetWorld()->GetTimeSeconds();

	if (auto* UIMgr = UUIManagerSubsystem::Get(this))
	{
		if (InitialUITag.IsValid())
		{
			UIMgr->HideUI(InitialUITag);
		}

		if (LoadingUITag.IsValid())
		{
			UIMgr->ShowUI(LoadingUITag);
		}
	}
	
	RequestNextRound();
}

void AGuesserPlayerController::OnGameEnd()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] OnGameEnd -> Show Loading"));
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[PC] OnGameEnd failed: no World")); return; }

	// Clear any pending loading timer (e.g. if EndGame races with data load)
	GetWorld()->GetTimerManager().ClearTimer(LoadingTimerHandle);

	LoadingStartTime = GetWorld()->GetTimeSeconds();

	if (auto* UIMgr = UUIManagerSubsystem::Get(this))
	{
		if (GameHUDTag.IsValid())
		{
			UIMgr->HideUI(GameHUDTag);
		}

		if (LoadingUITag.IsValid())
		{
			UIMgr->ShowUI(LoadingUITag);
		}
	}

	const float MinTime = UUIManagerSettings::Get()->MinLoadingTime;
	GetWorld()->GetTimerManager().SetTimer(
		LoadingTimerHandle, this, &AGuesserPlayerController::ReturnToMain, MinTime, false);
}

void AGuesserPlayerController::OnOperatorDataReady(const FOperatorImage& Tex, const TArray<FString>& Hints)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] OnOperatorDataReady | Hints=%d"), Hints.Num());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[PC] OnOperatorDataReady failed: no World")); return; }

	const float Elapsed = GetWorld()->GetTimeSeconds() - LoadingStartTime;
	const float MinTime = UUIManagerSettings::Get()->MinLoadingTime;
	const float Remaining = MinTime - Elapsed;

	if (Remaining > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			LoadingTimerHandle, this, &AGuesserPlayerController::FinishLoading, Remaining, false);
	}
	else
	{
		FinishLoading();
	}
}

void AGuesserPlayerController::FinishLoading()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] FinishLoading -> Show GameHUD"));
	if (auto* UIMgr = UUIManagerSubsystem::Get(this))
	{
		if (LoadingUITag.IsValid())
		{
			UIMgr->HideUI(LoadingUITag);
		}

		if (GameHUDTag.IsValid())
		{
			UIMgr->ShowUI(GameHUDTag);
		}
	}
}

void AGuesserPlayerController::ReturnToMain()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] ReturnToMain -> Show MainUI"));
	if (auto* UIMgr = UUIManagerSubsystem::Get(this))
	{
		if (LoadingUITag.IsValid())
		{
			UIMgr->HideUI(LoadingUITag);
		}

		if (InitialUITag.IsValid())
		{
			UIMgr->ShowUI(InitialUITag);
		}
	}
}

void AGuesserPlayerController::RequestNextRound_Implementation()
{
	UE_LOG(LogArknights, Log, TEXT("[PC] RequestNextRound"));
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!GM) { UE_LOG(LogArknights, Warning, TEXT("[PC] RequestNextRound failed: no GameMode")); return; }
	GM->StartNewRound();

}

void AGuesserPlayerController::NetMulticast_SyncGameSettings_Implementation(int32 InDefaultLevel, int32 InShuffleLimit, int32 InMaxGuessCount, int32 InHintFrequency)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] NetMulticast_SyncGameSettings | Level=%d | Shuffle=%d | MaxGuess=%d | HintFreq=%d"),
		InDefaultLevel, InShuffleLimit, InMaxGuessCount, InHintFrequency);

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->NetSync_DefaultLevel(InDefaultLevel);
		Subsystem->NetSync_ShuffleLimit(InShuffleLimit);
		Subsystem->NetSync_MaxGuessCount(InMaxGuessCount);
		Subsystem->NetSync_HintFrequency(InHintFrequency);
	}
}
