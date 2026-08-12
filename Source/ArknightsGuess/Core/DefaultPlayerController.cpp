// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/UI/UIManagerSettings.h"
#include "ArknightsGuess/UI/UIManagerSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ArknightsGuess.h"
#include "GuessGame/GuessGameSettings.h"
#include "Kismet/GameplayStatics.h"

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

	// Bind loading-flow delegates
	if (auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnGuessGameStart.AddDynamic(this, &ADefaultPlayerController::OnGameStart);
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
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] OnGameStart -> Travel to game level"));

	// Travel to the game level — mode was already stored by Subsystem->StartUp
	auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto* Settings = UGuessGameSettings::Get();
	const FGameplayTag Mode = Sub ? Sub->GetGameplayMode() : FGameplayTag();
	if (Settings && Mode.IsValid() && Settings->ModeLevels.Contains(Mode))
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, Settings->ModeLevels[Mode], false);
	}

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
	auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	auto Settings = UGuessGameSettings::Get();
	if (!Subsystem || !Settings) { UE_LOG(LogArknights, Warning, TEXT("[PC] StartGame failed: no Settings or Subsystem")); return; }
	if (!Settings->ModeLevels.Contains(Mode)) { UE_LOG(LogArknights, Warning, TEXT("[PC] StartGame failed: Can't find level to open")); return; }

	Subsystem->StartUp(Mode);
}


void ADefaultPlayerController::Server_UpdateGameSetting_Implementation(const FGameplayTag& SettingTag, const FString& Value)
{
	if (!IsLocalController()) return;

	auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Sub) return;

	const int32 IntValue = FCString::Atoi(*Value);

	if (SettingTag == SettingTags::DefaultLevel())
		Sub->SetDefaultLevel(IntValue);
	else if (SettingTag == SettingTags::ShuffleLimit())
		Sub->SetShuffleLimit(IntValue);
	else if (SettingTag == SettingTags::MaxGuessCount())
		Sub->SetMaxGuessCount(IntValue);
	else if (SettingTag == SettingTags::HintFrequency())
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
	Sub->NetSync_Setting(SettingTag, IntValue);
}