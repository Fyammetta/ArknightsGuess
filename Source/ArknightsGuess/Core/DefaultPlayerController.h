// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
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
	void PrepareForMultiply(const FString& Port);

	UFUNCTION(BlueprintCallable)
	void JoinLocalServer(const FString& Port);
};
