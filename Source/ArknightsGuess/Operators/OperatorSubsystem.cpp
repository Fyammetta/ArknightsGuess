// Fill out your copyright notice in the Description page of Project Settings.

#include "OperatorSubsystem.h"
#include "OperatorTypes.h"
#include "OperatorUISettings.h"
#include "ArknightsGuess/Core/GuessGameStateBase.h"
#include "ArknightsGuess.h"

UDataTable* UOperatorSubsystem::GetCachedDataTable()
{
	if (!OperatorDataTable)
	{
		if (auto* Settings = UOperatorUISettings::Get())
		{
			OperatorDataTable = Settings->OperatorDatas.LoadSynchronous();
		}
	}
	return OperatorDataTable.Get();
}

void UOperatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UsedOperators.Empty();
	SpareOperators.Empty();
	IsGameRunning = false;

	if (auto Settings = UOperatorUISettings::Get())
	{
		ShuffleLimit = Settings->ShuffleLimit;
		MaxGuessCount = Settings->MaxGuessCount;
		HintFrequency = Settings->HintFrequency;
		SetDefaultLevel(Settings->DefaultLevel);
	}
}

void UOperatorSubsystem::Deinitialize()
{
	Super::Deinitialize();
	EndGame();
}

void UOperatorSubsystem::StartUp(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[Subsystem] StartUp | Mode=%s | IsServer=%d"), *Mode.ToString(), IsRunningOnServer());
	if (IsGameRunning) { UE_LOG(LogArknights, Warning, TEXT("[Subsystem] StartUp ignored: game already running")); return; }

	if (UDataTable* DataTable = GetCachedDataTable())
	{
		if (IsRunningOnServer())
		{
			SpareOperators.Empty();
			DataTable->ForeachRow<FOperatorDataRow>(TEXT("OperatorSubsystem::StartUp"),
				[&List = SpareOperators, &Set = OperatorNames](const FName& RowName, const FOperatorDataRow& Row)
				{
					List.Append(FOperatorData::MakeFromDataRow(RowName, Row));
					Set.Add(FOperatorNamePair(RowName, Row.AvailableNames));

				});
		}
		else
		{
			OperatorNames.Empty();
			DataTable->ForeachRow<FOperatorDataRow>(TEXT("OperatorSubsystem::StartUp"),
				[&List = OperatorNames](const FName& RowName, const FOperatorDataRow& Row)
				{
					List.Add(FOperatorNamePair(RowName, Row.AvailableNames));
				});
		}
	}

	GuessMode = Mode;
	IsGameRunning = true;
	OnGuessGameStart.Broadcast();
}

void UOperatorSubsystem::EndGame()
{
	UE_LOG(LogArknights, Log, TEXT("[Subsystem] EndGame | IsServer=%d"), IsRunningOnServer());
	if (!IsGameRunning) return;

	UsedOperators.Empty();
	SpareOperators.Empty();
	IsGameRunning = false;
	OnGuessGameEnd.Broadcast();
}

TSet<FOperatorNamePair> UOperatorSubsystem::GetAllOperatorNames() const
{
	return OperatorNames;
}

UMaterialInstanceDynamic* UOperatorSubsystem::GetDynamicMaterial()
{
	auto* Settings = UOperatorUISettings::Get();
	if (!Settings) { UE_LOG(LogArknights, Warning, TEXT("[Subsystem] GetDynamicMaterial failed: no Settings")); return nullptr; }

	if (!MaterialBase && Settings->Materials.Contains(GuessMode))
	{
		MaterialBase = Settings->Materials[GuessMode].LoadSynchronous();
	}

	if (!MaterialBase) return nullptr;

	return UMaterialInstanceDynamic::Create(MaterialBase, this);;
}

void UOperatorSubsystem::SetDefaultLevel(int32 Level)
{
	if (!IsRunningOnServer() || IsGameRunning || Level <= 0)
		return;
	DefaultLevel = Level;
}

int32 UOperatorSubsystem::GetDefaultLevel() const
{
	return DefaultLevel;
}

void UOperatorSubsystem::SetShuffleLimit(int32 Limit)
{
	if (!IsRunningOnServer() || IsGameRunning || Limit <= 0)
		return;
	ShuffleLimit = Limit;
}

int32 UOperatorSubsystem::GetShuffleLimit() const
{
	return ShuffleLimit;
}

void UOperatorSubsystem::SetMaxGuessCount(int32 Limit)
{
	if (!IsRunningOnServer() || IsGameRunning || Limit <= 0) return;
	MaxGuessCount = Limit;
}

int32 UOperatorSubsystem::GetMaxGuessCount() const
{
	return MaxGuessCount;
}

void UOperatorSubsystem::SetHintFrequency(int32 Freq)
{
	if (!IsRunningOnServer() || IsGameRunning || Freq <= 0)
		return;
	HintFrequency = Freq;
}

int32 UOperatorSubsystem::GetHintFrequency() const
{
	return HintFrequency;
}

FOperatorData UOperatorSubsystem::GetRandomOperatorData()
{
	UE_LOG(LogArknights, Log, TEXT("[Subsystem] GetRandomOperatorData | Spare=%d | Used=%d"), SpareOperators.Num(), UsedOperators.Num());
	if (!IsRunningOnServer() || SpareOperators.IsEmpty()) { UE_LOG(LogArknights, Warning, TEXT("[Subsystem] GetRandomOperatorData failed: not server or no operators")); return FOperatorData(); }

	if (ShuffleLimit <= 0)
	{
		int32 Random = FMath::RandRange(0, SpareOperators.Num() - 1);
		SpareOperators.Swap(SpareOperators.Num() - 1, Random);
		return SpareOperators.Pop();
	}

	if (UsedOperators.Num() >= ShuffleLimit || SpareOperators.IsEmpty())
		SpareOperators.Append(MoveTemp(UsedOperators));

	int32 Random = FMath::RandRange(0, SpareOperators.Num() - 1);
	SpareOperators.Swap(SpareOperators.Num() - 1, Random);
	FOperatorData Data = SpareOperators.Pop();
	UsedOperators.Add(Data);
	return Data;
}

void UOperatorSubsystem::NetSync_DefaultLevel(int32 Level)
{
	DefaultLevel = Level;
}

void UOperatorSubsystem::NetSync_ShuffleLimit(int32 Limit)
{
	ShuffleLimit = Limit;
}

void UOperatorSubsystem::NetSync_MaxGuessCount(int32 Count)
{
	MaxGuessCount = Count;
}

void UOperatorSubsystem::NetSync_HintFrequency(int32 Freq)
{
	HintFrequency = Freq;
}

bool UOperatorSubsystem::IsRunningOnServer() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetNetMode() < ENetMode::NM_Client;
	}
	return false;
}
