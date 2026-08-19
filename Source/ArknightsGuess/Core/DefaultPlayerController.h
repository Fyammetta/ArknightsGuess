// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "OnlineSessionSettings.h"
#include "LanDiscoverySubsystem.h"
#include "DefaultPlayerController.generated.h"

struct FOperatorImage;

/**
 * Base player controller — UI lifecycle, loading transitions, and all game-control RPCs.
 * AGuesserPlayerController inherits from this and is used as the in-game PC class.
 */
UCLASS()
class ARKNIGHTSGUESS_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	// TSharedRef 无默认构造，不能做 UObject 成员；TSharedPtr 可空，构造安全
	TSharedPtr<FOnlineSessionSearch> Search;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnGameStart();

	virtual void InitPlayerState() override;
public:
	// ---- Game-control RPCs ----
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void StartGame(const FGameplayTag& Mode);


	UFUNCTION(Reliable, Server, BlueprintCallable)
	void Server_UpdateGameSetting(const FGameplayTag& SettingTag, const FString& Value);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateGameSetting(const FGameplayTag& SettingTag, const FString& Value);
	
	UFUNCTION(BlueprintCallable)
	void PrepareForMultiply(const FString& RoomName, const FString& Port);

	UFUNCTION(BlueprintCallable)
	void JoinServer(const FString& Url);
	
	void JoinServer(const FOnlineSessionSearchResult& Session);
	
	bool TryFindLocalServer(FOnFindSessionsCompleteDelegate&& Delegate, FDelegateHandle& OutHandle);
	
	const TArray<FOnlineSessionSearchResult>& GetAllSessions() const;

	/** 方案3:自建 UDP 局域网发现(热点下可用)。 */
	UFUNCTION(BlueprintCallable)
	bool TryFindLocalServerDirect();

	/** 方案3:直连一个自建发现到的房间(用 BestIP:GamePort)。 */
	void JoinDiscoveredRoom(const FLanRoomInfo& Room);
};
