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
#include "IPAddress.h"
#include "SocketSubsystem.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "GuessGame/GuessGameSettings.h"
#include "GuessGamerSettings.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PlayerIconInterface.h"
#include "LanDiscoverySubsystem.h"

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
	// 保留旧 PlayerState 的图标(重置/重连场景下迁移)
	UTexture2D* OldIcon = nullptr;
	if (auto* IconInterface = Cast<IPlayerIconInterface>(PlayerState))
	{
		OldIcon = IconInterface->GetPlayerIcon();
	}

	Super::InitPlayerState();

	if (OldIcon)
	{
		if (auto* IconInterface = Cast<IPlayerIconInterface>(PlayerState))
		{
			IconInterface->ChangePlayerIcon(OldIcon);
		}
	}

	// 服务器端:本机(房主)的 PlayerState 刚创建且图标为空时,用本机设置初始化
	if (HasAuthority())
	{
		if (auto* PS = GetPlayerState<AGuessPlayerState>())
		{
			if (!PS->GetPlayerIcon())
			{
				PS->SetPlayerName(UGuessGamerSettings::GetPlayerName());
				PS->ChangePlayerIcon(UGuessGamerSettings::GetPlayerIcon());
			}
		}
	}
}

void ADefaultPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 客户端:自己的 PlayerState 复制到位后,把本机设置上报给服务器
	// (服务器 spawn 的 PlayerState 不再读房主设置,头像/名字必须由玩家自己提供)
	if (!HasAuthority())
	{
		UTexture2D* Icon = UGuessGamerSettings::GetPlayerIcon();
		Server_SetPlayerInfo(UGuessGamerSettings::GetPlayerName(), FSoftObjectPath(Icon));
	}
}

void ADefaultPlayerController::Server_SetPlayerInfo_Implementation(const FString& NewName, const FSoftObjectPath& NewIcon)
{
	if (auto* PS = GetPlayerState<AGuessPlayerState>())
	{
		PS->SetPlayerName(NewName);
		PS->ChangePlayerIcon(NewIcon);
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

	// 方案3:自建 UDP 广播广告(绑定 0.0.0.0,安卓热点下 PC 也能搜到)
	if (auto* Lan = GetGameInstance()->GetSubsystem<ULanDiscoverySubsystem>())
	{
		Lan->StartAdvertising(RoomName, WorldUrl.Port);
	}

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
	SessionPtr->AddOnCreateSessionCompleteDelegate_Handle(Delegate);
	SessionPtr->CreateSession(*Local, FName(RoomName), Settings);
	//SessionPtr->StartSession(FName(RoomName));
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
	SessionPtr->JoinSession(1,FName(RoomName),Session);
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

bool ADefaultPlayerController::TryFindLocalServerDirect()
{
	if (auto* Lan = GetGameInstance()->GetSubsystem<ULanDiscoverySubsystem>())
	{
		return Lan->StartSearching();
	}
	UE_LOG(LogArknights, Warning, TEXT("[PC] TryFindLocalServerDirect: LanDiscoverySubsystem missing"));
	return false;
}

void ADefaultPlayerController::JoinDiscoveredRoom(const FLanRoomInfo& Room)
{
	FString Host = Room.BestIP;
	if (Host.IsEmpty() && Room.IPs.Num() > 0)
	{
		Host = Room.IPs[0];
	}
	if (Host.IsEmpty())
	{
		UE_LOG(LogArknights, Warning, TEXT("[PC] JoinDiscoveredRoom: no IP for room '%s'"), *Room.RoomName);
		return;
	}

	const FString Url = FString::Printf(TEXT("%s:%d"), *Host, Room.GamePort > 0 ? Room.GamePort : 7777);
	UE_LOG(LogArknights, Log, TEXT("[PC] JoinDiscoveredRoom | Room=%s | Url=%s"), *Room.RoomName, *Url);
	ClientTravel(Url, TRAVEL_Absolute);
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