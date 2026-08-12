// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OperatorTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "OperatorSubsystem.generated.h"

struct FOperatorData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGuessRoundStateChangeDelegate, EGuessRoundState, RoundState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOperatorDataReceiveDelegate, const FOperatorImage&, Tex, const TArray<FString>&, Hints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGuessProcessChangeDelegate, int32, Round, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGuessGameStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGuessGameEndDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisplayNextHintsDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameSettingChangedDelegate, FGameplayTag, SettingTag, const FString&, NewValue);

UCLASS()
class ARKNIGHTSGUESS_API UOperatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMaterial> MaterialBase;

	UPROPERTY()
	TObjectPtr<UDataTable> OperatorDataTable;

	TArray<FOperatorData> SpareOperators;
	TArray<FOperatorData> UsedOperators;
	TSet<FOperatorNamePair> OperatorNames;

	FGameplayTag GuessMode;
	int32 DefaultLevel;
	int32 ShuffleLimit;
	int32 MaxGuessCount;
	int32 HintFrequency;

	FVector2D MosaicOffset;

	bool bIsGameRunning;

	int32 ExpectedPlayerCount = 1;

public:
	// ---  Default ---
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; };
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Delegates (bind UI / other systems here) ----
	FGuessRoundStateChangeDelegate OnGuessRoundStateChanged;
	FOperatorDataReceiveDelegate OnOperatorDataReceived;
	FGuessProcessChangeDelegate OnGuessProcessChanged;
	FGuessGameStartDelegate OnGuessGameStart;
	FGuessGameEndDelegate OnGuessGameEnd;
	FDisplayNextHintsDelegate OnNextHintsDisplayAllowed;
	FOnGameSettingChangedDelegate OnGameSettingChanged;

	UFUNCTION(BlueprintCallable)
	void StartUp(const FGameplayTag& Mode);

	UFUNCTION(BlueprintCallable)
	void EndGame();

	UFUNCTION(BlueprintCallable)
	TSet<FOperatorNamePair> GetAllOperatorNames() const { return OperatorNames; }

	UMaterialInstanceDynamic* GetDynamicMaterial();

	UFUNCTION(BlueprintCallable)
	void SetDefaultLevel(int32 Level);

	UFUNCTION(BlueprintCallable)
	int32 GetDefaultLevel() const { return DefaultLevel; }

	UFUNCTION(BlueprintCallable)
	void SetShuffleLimit(int32 Limit);

	UFUNCTION(BlueprintCallable)
	int32 GetShuffleLimit() const { return ShuffleLimit; }

	UFUNCTION(BlueprintCallable)
	void SetMaxGuessCount(int32 Limit);

	UFUNCTION(BlueprintCallable)
	int32 GetMaxGuessCount() const { return MaxGuessCount; }

	UFUNCTION(BlueprintCallable)
	void SetHintFrequency(int32 Freq);

	UFUNCTION(BlueprintCallable)
	int32 GetHintFrequency() const { return HintFrequency; }

	UFUNCTION(BlueprintCallable)
	FOperatorData GetRandomOperatorData();

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetGameplayMode() const { return GuessMode; }

	UFUNCTION(BlueprintCallable)
	bool IsRunningOnServer() const;

	UFUNCTION(BlueprintCallable)
	bool IsGameRunning() const { return bIsGameRunning; }

	UFUNCTION(BlueprintCallable)
	int32 GetExpectedPlayerCount() const { return ExpectedPlayerCount; }

	UFUNCTION(BlueprintCallable)
	void SetExpectedPlayerCount(int32 Count) { ExpectedPlayerCount = FMath::Max(1, Count); }

	// ---- Net sync helpers (no server guard, called from PC NetMulticast) ----
	void NetSync_Setting(const FGameplayTag& SettingTag, int32 Value);

private:
	UDataTable* GetCachedDataTable();
};