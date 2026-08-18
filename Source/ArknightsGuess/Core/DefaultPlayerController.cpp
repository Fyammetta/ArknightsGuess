// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayerController.h"

#include "GuessGameModeBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/UI/UIManagerSettings.h"
#include "ArknightsGuess/UI/UIManagerSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ArknightsGuess.h"
#include "DevNotificationSubsystem.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "GuessGame/GuessGameSettings.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PlayerIconInterface.h"

// ============================================================
//  UI lifecycle
// ============================================================

void ADefaultPlayerController::BeginPlay()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultPC] BeginPlay"));
	Super::BeginPlay();

	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this);
	SetShowMouseCursor(true);

	// Audio setup handled by UGuessGameInstance::OnStart

	// Default loading / HUD tags (blueprint can override)

	// Bind loading-flow delegates
	if (auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnGuessGameStart.AddDynamic(this, &ADefaultPlayerController::OnGameStart);
	}
	Search = MakeShared<FOnlineSessionSearch>();
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
}

void ADefaultPlayerController::InitPlayerState()
{
	UTexture2D* Icon = nullptr;
	if (auto* IconInterface = Cast<IPlayerIconInterface>(PlayerState))
	{
		Icon = IconInterface->GetPlayerIcon();
	}

	Super::InitPlayerState();

	if (Icon)
	{
		if (auto* IconInterface = Cast<IPlayerIconInterface>(PlayerState))
		{
			IconInterface->ChangePlayerIcon(Icon);
		}
	}
}

void ADefaultPlayerController::PrepareForMultiply(const FString& RoomName, const FString& Port)
{
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	FUniqueNetIdRepl Local = GetLocalPlayer()->GetPreferredUniqueNetId();
	if (!SessionPtr.IsValid() || !Local.IsValid()) return;
	
	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = true;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = false;
	Settings.bAllowJoinInProgress = false;
	Settings.NumPublicConnections = 15;
	
	Settings.Set(TEXT("RoomName"), RoomName , EOnlineDataAdvertisementType::ViaOnlineService);

	
	FOnCreateSessionCompleteDelegate Delegate = FOnCreateSessionCompleteDelegate::CreateWeakLambda(
	this, 
	[World = TWeakObjectPtr<UWorld>(GetWorld())](FName,bool)
	{
		auto* Settings = UGuessGameSettings::Get();
		auto Tag = MapTags::MultiRoom();
		if (World.IsValid() && Settings && Settings->ModeLevels.Contains(Tag))
		{
			auto Map = Settings->ModeLevels[Tag];
	
			World->SeamlessTravel(Map.LoadSynchronous()->GetMapName(),true);
		}
	});
	FURL WorldUrl = GetWorld()->URL;
	if (!Port.IsEmpty())
		WorldUrl.Port = FCString::Atoi(*Port);
	if (	GetWorld()->Listen(WorldUrl))
	{
		SessionPtr->AddOnCreateSessionCompleteDelegate_Handle(Delegate);
		SessionPtr->CreateSession(*Local, NAME_GameSession, Settings);
	}
	else
	{
		UDevNotificationSubsystem::Get(this)->ShowNotification(TEXT("Invalid Port, change it and try again"));
	}

}

void ADefaultPlayerController::JoinServer(const FString& Url)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] JoinServer | Url=%s"), *Url);
	
	ClientTravel(Url, TRAVEL_Absolute);
}

void ADefaultPlayerController::JoinServer(const FOnlineSessionSearchResult& Session)
{
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	JoinServer(Session.GetSessionIdStr());
}

bool ADefaultPlayerController::TryFindLocalServer(FOnFindSessionsCompleteDelegate&& Delegate, FDelegateHandle& OutHandle)
{
	if (!Delegate.IsBound()) return false;
	
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	if (!Search.IsValid())
		Search = MakeShared<FOnlineSessionSearch>();
	Search->bIsLanQuery = true;
	Search->MaxSearchResults = 10;

	FUniqueNetIdRepl Local = GetLocalPlayer()->GetPreferredUniqueNetId();
	if (!OutHandle.IsValid())
		OutHandle = SessionPtr->AddOnFindSessionsCompleteDelegate_Handle(Delegate);

	return SessionPtr->FindSessions(*Local, Search.ToSharedRef());
}

const TArray<FOnlineSessionSearchResult>& ADefaultPlayerController::GetAllSessions() const
{
	if (!Search.IsValid())
	{
		static const TArray<FOnlineSessionSearchResult> EmptyResults;
		return EmptyResults;
	}
	return Search->SearchResults;
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