// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayerController.h"
#include "GuessGameModeBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/UI/UIManagerSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ArknightsGuess.h"
#include "DefaultGameStateBase.h"
#include "DevNotificationSubsystem.h"
#include "IPAddress.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "GuessGame/GuessGameSettings.h"
#include "GameFramework/PlayerState.h"
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
	
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();

	SessionPtr->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateWeakLambda(this,
		[this](bool){
			if (auto System = UUIManagerSubsystem::Get(this))
			{
				System->HideUI(UITags::Waiting());
			}
		}));
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
		if (GetWorld())
			GetWorld()->ServerTravel(Settings->ModeLevels[Mode].ToSoftObjectPath().GetLongPackageName(),false);
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
	FURL WorldUrl = GetWorld()->URL;
	WorldUrl.Port = Port.IsEmpty() ? 7777 : FCString::Atoi(*Port);
	if (WorldUrl.Port <= 0 || WorldUrl.Port > 65535)
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] PrepareForMultiply: invalid port '%s'"), *Port);
		UDevNotificationSubsystem::Get(this)->ShowNotification(TEXT("Invalid Port, change it and try again"));
		return;
	}

	// 先 Listen:不依赖 OSS,确保端口一定在监听
	if (!GetWorld()->Listen(WorldUrl))
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] Listen FAILED | Port=%d | Check firewall / port in use"), WorldUrl.Port);
		UDevNotificationSubsystem::Get(this)->ShowNotification(TEXT("Listen failed, check port or firewall"));
		return;
	}
	UE_LOG(LogArknights, Log, TEXT("[PC] Listen OK | Port=%d | Room=%s"), WorldUrl.Port, *RoomName);

	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	FUniqueNetIdRepl Local = GetLocalPlayer()->GetPreferredUniqueNetId();
	if (!SessionPtr.IsValid() || !Local.IsValid())
	{
		// 监听已生效,OSS 失败只影响"房间广播/搜索",不影响客户端用 IP 直连加入
		UE_LOG(LogArknights, Warning, TEXT("[PC] OSS unavailable (SessionPtr valid=%d, Local valid=%d): LAN advertise skipped, direct IP join still works"),
			SessionPtr.IsValid(), Local.IsValid());
		return;
	}

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = true;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = false;
	Settings.bAllowJoinInProgress = false;
	Settings.NumPublicConnections = 16;
	
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

			World->ServerTravel(Map.ToSoftObjectPath().GetLongPackageName() + TEXT("?listen"),true);
		}
	});
	SessionPtr->AddOnCreateSessionCompleteDelegate_Handle(Delegate);
	// 统一使用 NAME_GameSession，与 JoinSession 及 GameMode::PostLogin 的注册保持一致。
	// 注册由 GameMode::PostLogin(NAME_GameSession, ...) 统一处理，此处不再重复 RegisterPlayer。
	SessionPtr->CreateSession(*Local, NAME_GameSession, Settings);
	if (auto System = UUIManagerSubsystem::Get(this))
	{
		System->ShowUI(UITags::Loading());
	}
	SessionPtr->StartSession(NAME_GameSession);
}

void ADefaultPlayerController::QuitServer()
{
	auto Settings = UGuessGameSettings::Get();
	auto World = GetWorld();
	if (!Settings || !World || !Settings->ModeLevels.Contains(MapTags::Main())) return;
	auto Map = Settings->ModeLevels[MapTags::Main()];
	if (Map.IsNull())
		return;
	
	FString MapName = Map.ToSoftObjectPath().GetLongPackageName();
	if (HasAuthority())
	{
		IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
		SessionPtr->DestroySession(NAME_GameSession);
		
	}
	JoinServer(MapName);

}

void ADefaultPlayerController::JoinServer(const FString& Url)
{
	UE_LOG(LogArknights, Log, TEXT("[PC] JoinServer | Url=%s"), *Url);

	// 补全缺省端口,避免客户端连到端口 0
	FString TravelUrl = Url;
	if (!TravelUrl.Contains(TEXT(":")))
	{
		TravelUrl += TEXT(":7777");
	}
	ClientTravel(TravelUrl, TRAVEL_Absolute);
}

void ADefaultPlayerController::JoinServer(const FOnlineSessionSearchResult& Session)
{
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	FString RoomName{};
	Session.Session.SessionSettings.Get(TEXT("RoomName"), RoomName);
	FString Info{};
	SessionPtr->GetResolvedConnectString(Session,NAME_GamePort,Info);
	UE_LOG(LogArknights, Log, TEXT("[PC] JoinServer(session) | Room=%s | Connect=%s"), *RoomName, *Info);
	if (Info.IsEmpty())
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] JoinServer(session): empty connect string, fallback to session id"));
		Info = Session.GetSessionIdStr();
	}
	SessionPtr->JoinSession(1,NAME_GameSession,Session);
	FUniqueNetIdRepl Local = GetLocalPlayer()->GetPreferredUniqueNetId();

	// 与 JoinSession 使用相同的 NAME_GameSession，服务器 PostLogin 也会基于此注册
	//SessionPtr->RegisterPlayer(NAME_GameSession, *Local, false);

	ClientTravel(Info, TRAVEL_Absolute);

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

	if (auto System = UUIManagerSubsystem::Get(this))
	{
		System->ShowUI(UITags::Waiting());
	}
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

bool ADefaultPlayerController::IsAllPlayerReady()
{
	auto World = GetWorld();
	if (!World) return false;
	auto GS = World->GetGameState();
	auto GM = World->GetAuthGameMode<AGuessGameModeBase>();
	
	if (!GS || !GM) return false;
	return GM->GetReadyPlayerCount() == GS->PlayerArray.Num() - 1;
}

void ADefaultPlayerController::PreparedForStart_Implementation()
{
	auto World = GetWorld();
	if (!World) return;
	auto GM = World->GetAuthGameMode<AGuessGameModeBase>();
	
	if (!GM) return;
	GM->SetPlayerPrepared(this);
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
	if (auto GM = GetWorld() ? GetWorld()->GetAuthGameMode<AGuessGameModeBase>() : nullptr)
		GM->ResetPreparedPlayers();
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

	if (auto GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameStateBase>() : nullptr)
		GS->NetMulticast_UpdateGameSetting(SettingTag, Value);
}

