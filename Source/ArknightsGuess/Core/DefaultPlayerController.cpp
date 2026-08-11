// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess/UI/UIManagerSettings.h"
#include "ArknightsGuess/UI/UIManagerSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ArknightsGuess.h"

// ============================================================
//  UI lifecycle
// ============================================================

void ADefaultPlayerController::BeginPlay()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] BeginPlay"));
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
		Sub->OnGuessGameStart.AddDynamic(this, &ADefaultPlayerController::OnGameStart);
		Sub->OnOperatorDataReceived.AddDynamic(this, &ADefaultPlayerController::OnOperatorDataReady);
		Sub->OnGuessGameEnd.AddDynamic(this, &ADefaultPlayerController::OnGameEnd);
	}

	if (InitialUITag.IsValid())
	{
		if (auto* UIMgr = UUIManagerSubsystem::Get(this))
		{
			UIMgr->ShowUI(InitialUITag);
		}
	}
}

void ADefaultPlayerController::OnGameStart()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnGameStart -> Show Loading"));
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

	const float MinTime = UUIManagerSettings::Get()->MinLoadingTime;
	GetWorld()->GetTimerManager().SetTimer(
		LoadingTimerHandle, this, &ADefaultPlayerController::FinishLoading, MinTime, false);
}

void ADefaultPlayerController::OnGameEnd()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnGameEnd -> Show Loading"));
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[DefaultPC] OnGameEnd failed: no World")); return; }

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
		LoadingTimerHandle, this, &ADefaultPlayerController::ReturnToMain, MinTime, false);
}

void ADefaultPlayerController::OnOperatorDataReady(const FOperatorImage& Tex, const TArray<FString>& Hints)
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnOperatorDataReady | Hints=%d"), Hints.Num());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[DefaultPC] OnOperatorDataReady failed: no World")); return; }

	const float Elapsed = GetWorld()->GetTimeSeconds() - LoadingStartTime;
	const float MinTime = UUIManagerSettings::Get()->MinLoadingTime;
	const float Remaining = MinTime - Elapsed;

	if (Remaining > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			LoadingTimerHandle, this, &ADefaultPlayerController::FinishLoading, Remaining, false);
	}
	else
	{
		FinishLoading();
	}
}

void ADefaultPlayerController::FinishLoading()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] FinishLoading -> Show GameHUD"));
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

void ADefaultPlayerController::ReturnToMain()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] ReturnToMain -> Show MainUI"));
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

// ============================================================
//  Game-control RPCs
// ============================================================

void ADefaultPlayerController::StartGame_Implementation(const FGameplayTag& Mode)
{
	if (!IsLocalController())
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] StartGame rejected: host only"));
		return;
	}

	UE_LOG(LogArknights, Log, TEXT("[PC] StartGame | Mode=%s"), *Mode.ToString());
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto GM = GetWorld()->GetAuthGameMode<AGuessGameModeBase>();
	if (!Subsystem || !GM) { UE_LOG(LogArknights, Warning, TEXT("[PC] StartGame failed: no Subsystem or GameMode")); return; }

	GM->StartGame(Mode);
	Subsystem->StartUp(Mode);
}

void ADefaultPlayerController::EndGame_Implementation()
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
}

void ADefaultPlayerController::Server_UpdateGameSetting_Implementation(const FGameplayTag& SettingTag, const FString& Value)
{
	if (!IsLocalController()) return;

	auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Sub) return;

	const int32 IntValue = FCString::Atoi(*Value);

	if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.DefaultLevel"))
		Sub->SetDefaultLevel(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.ShuffleLimit"))
		Sub->SetShuffleLimit(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.MaxGuessCount"))
		Sub->SetMaxGuessCount(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.HintFrequency"))
		Sub->SetHintFrequency(IntValue);
	else
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] Server_UpdateGameSetting: unknown tag %s"), *SettingTag.ToString());
		return;
	}

	NetMulticast_UpdateGameSetting(SettingTag, Value);
}

void ADefaultPlayerController::NetMulticast_UpdateGameSetting_Implementation(const FGameplayTag& SettingTag, const FString& Value)
{
	auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Sub) return;

	const int32 IntValue = FCString::Atoi(*Value);

	if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.DefaultLevel"))
		Sub->NetSync_DefaultLevel(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.ShuffleLimit"))
		Sub->NetSync_ShuffleLimit(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.MaxGuessCount"))
		Sub->NetSync_MaxGuessCount(IntValue);
	else if (SettingTag == FGameplayTag::RequestGameplayTag("Settings.HintFrequency"))
		Sub->NetSync_HintFrequency(IntValue);
	else
		UE_LOG(LogArknights, Warning, TEXT("[PC] NetMulticast_UpdateGameSetting: unknown tag %s"), *SettingTag.ToString());
}
