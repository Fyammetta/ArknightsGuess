// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GuesserPlayerController.generated.h"

/**
 *
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuesserPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FGameplayTag InitialUITag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FGameplayTag LoadingUITag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FGameplayTag GameHUDTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float LoadingMinTime = 2.f;

	UFUNCTION()
	void OnGameStart();

	UFUNCTION()
	void OnGameEnd();

	UFUNCTION()
	void OnOperatorDataReady(const FOperatorImage& Tex, const TArray<FString>& Hints);

	void FinishLoading();
	void ReturnToMain();

	float LoadingStartTime = 0.0f;
	FTimerHandle LoadingTimerHandle;

public:
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void StartGame(const FGameplayTag& Mode);
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void EndGame();
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void ConfirmAnswer(const FName& Answer);
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void RequestNextRound();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SyncGameSettings(int32 InDefaultLevel, int32 InShuffleLimit, int32 InMaxGuessCount, int32 InHintFrequency);

};
