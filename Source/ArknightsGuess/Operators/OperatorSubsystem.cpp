// Fill out your copyright notice in the Description page of Project Settings.

#include "OperatorSubsystem.h"
#include "OperatorTypes.h"
#include "OperatorUISettings.h"
#include "ArknightsGuess/Core/GuessGameStateBase.h"

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
	
	StartUp(FName("Mosaic"));
}

void UOperatorSubsystem::Deinitialize()
{
	Super::Deinitialize();
	EndGame();
}

void UOperatorSubsystem::StartUp(const FName& Mode)
{
	if (IsGameRunning) return;

	if (UDataTable* DataTable = GetCachedDataTable())
	{
		if (IsRunningOnServer())
		{
			SpareOperators.Empty();
			DataTable->ForeachRow<FOperatorDataRow>(TEXT("OperatorSubsystem::StartUp"),
				[&List = SpareOperators](const FName& RowName, const FOperatorDataRow& Row)
				{
					List.Append(FOperatorData::MakeFromDataRow(RowName, Row));
				});
		}
		OperatorNames.Append(DataTable->GetRowNames());
	}

	GuessMode = Mode;
	IsGameRunning = true;
	OnGuessGameStart.Broadcast();
}

void UOperatorSubsystem::EndGame()
{
	if (!IsGameRunning) return;

	UsedOperators.Empty();
	SpareOperators.Empty();
	OnGuessGameEnd.Broadcast();
}

TSet<FName> UOperatorSubsystem::GetAllOperatorNames() const
{
	return OperatorNames;
}

UMaterialInstanceDynamic* UOperatorSubsystem::GetDynamicMaterial(const FOperatorData& Data)
{
	auto* Settings = UOperatorUISettings::Get();
	if (!Settings) return nullptr;

	if (!MaterialBase && Settings->Materials.Contains(GuessMode))
	{
		MaterialBase = Settings->Materials[GuessMode].LoadSynchronous();
	}

	if (!MaterialBase) return nullptr;

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(MaterialBase, this);
	MID->SetTextureParameterValue(TEXT("Tex"), Data.Image.Texture.LoadSynchronous());
	return MID;
}

void UOperatorSubsystem::SetDefaultLevel(int32 Level)
{
	if (!IsRunningOnServer() || IsGameRunning || Level <= 0)
		return;
	DefaultLevel = Level * 4;
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
	if (!IsRunningOnServer() || SpareOperators.IsEmpty()) return FOperatorData();

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

bool UOperatorSubsystem::IsRunningOnServer() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetNetMode() < ENetMode::NM_Client;
	}
	return false;
}
