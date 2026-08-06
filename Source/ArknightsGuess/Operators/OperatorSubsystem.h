// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OperatorSubsystem.generated.h"

struct FOperatorData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGuessRoundStateChangeDelegate, EGuessRoundState, RoundState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOperatorDataReceiveDelegate, const FOperatorImage&, Tex, const TArray<FString>&, Hints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGuessProcessChangeDelegate, int32, Round, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGuessGameStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGuessGameEndDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisplayNextHintsDelegate);

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
	TSet<FName> OperatorNames;

	FName GuessMode;
	int32 DefaultLevel;
	int32 ShuffleLimit;
	int32 MaxGuessCount;
	int32 HintFrequency;

	bool IsGameRunning;

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

	UFUNCTION(BlueprintCallable)
	void StartUp(const FName& Mode);

	UFUNCTION(BlueprintCallable)
	void EndGame();

	UFUNCTION(BlueprintCallable)
	TSet<FName> GetAllOperatorNames() const;

	UMaterialInstanceDynamic* GetDynamicMaterial(const FOperatorData& Data);

	UFUNCTION(BlueprintCallable)
	void SetDefaultLevel(int32 Level);

	UFUNCTION(BlueprintCallable)
	int32 GetDefaultLevel() const;

	UFUNCTION(BlueprintCallable)
	void SetShuffleLimit(int32 Limit);

	UFUNCTION(BlueprintCallable)
	int32 GetShuffleLimit() const;

	UFUNCTION(BlueprintCallable)
	void SetMaxGuessCount(int32 Limit);

	UFUNCTION(BlueprintCallable)
	int32 GetMaxGuessCount() const;

	UFUNCTION(BlueprintCallable)
	void SetHintFrequency(int32 Freq);

	UFUNCTION(BlueprintCallable)
	int32 GetHintFrequency() const;

	UFUNCTION(BlueprintCallable)
	FOperatorData GetRandomOperatorData();

	UFUNCTION(BlueprintCallable)
	bool IsRunningOnServer() const;

private:
	UDataTable* GetCachedDataTable();
};
