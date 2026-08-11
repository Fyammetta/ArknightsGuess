// Fill out your copyright notice in the Description page of Project Settings.

#include "MosaicGuessComponent.h"
#include "GuessGameSettings.h"
#include "ArknightsGuess/Core/GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"

#include "Net/UnrealNetwork.h"
#include "ArknightsGuess.h"

UMosaicGuessComponent::UMosaicGuessComponent()
{
	SetIsReplicatedByDefault(true);
}

void UMosaicGuessComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMosaicGuessComponent, MosaicOffset);
}

void UMosaicGuessComponent::SetMosaicOffset(const FVector2D& Offset)
{
	UE_LOG(LogArknights, Log, TEXT("[MosaicComp] SetMosaicOffset | Offset=(%.2f, %.2f) | Authority=%d"), Offset.X, Offset.Y, GetOwner()->HasAuthority());
	MosaicOffset = Offset;
}

void UMosaicGuessComponent::OnNewRound()
{
	UE_LOG(LogArknights, Log, TEXT("[MosaicComp] OnNewRound"));
	const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
	const FVector2D Offset(FMath::RandRange(-Step, Step), FMath::RandRange(-Step, Step));
	SetMosaicOffset(Offset);

	// Direct broadcast for single-player / server: OnRep won't fire without replication
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnNewRound failed: no Subsystem")); return; }

	if (!Subsystem->IsRunningOnServer()) return;

	auto* GS = Cast<AGuessGameStateBase>(GetOwner());
	if (!GS) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnNewRound failed: no GameState")); return; }

	Subsystem->OnGuessProcessChanged.Broadcast(GS->GetRoundNumber(), Subsystem->GetDefaultLevel());
}

void UMosaicGuessComponent::OnWrongGuess(int32 GuessCount)
{
	UE_LOG(LogArknights, Log, TEXT("[MosaicComp] OnWrongGuess | GuessCount=%d"), GuessCount);
	const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
	const FVector2D Offset(FMath::RandRange(-Step, Step), FMath::RandRange(-Step, Step));
	SetMosaicOffset(Offset);

	// Direct broadcast for single-player / server: OnRep won't fire without replication
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnWrongGuess failed: no Subsystem")); return; }

	if (!Subsystem->IsRunningOnServer()) return;

	auto* GS = Cast<AGuessGameStateBase>(GetOwner());
	if (!GS) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnWrongGuess failed: no GameState")); return; }

	if (GuessCount % Step != 0)
	{
		Subsystem->OnGuessProcessChanged.Broadcast(GS->GetRoundNumber(), Subsystem->GetDefaultLevel() - GuessCount);
	}
}

void UMosaicGuessComponent::OnRep_MosaicOffset()
{
	UE_LOG(LogArknights, Log, TEXT("[MosaicComp] OnRep_MosaicOffset | Offset=(%.2f, %.2f)"), MosaicOffset.X, MosaicOffset.Y);
	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnRep_MosaicOffset failed: no Subsystem")); return; }

	auto* GS = Cast<AGuessGameStateBase>(GetOwner());
	if (!GS) { UE_LOG(LogArknights, Warning, TEXT("[MosaicComp] OnRep_MosaicOffset failed: no GameState")); return; }

	const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
	const int32 Count = GS->GetGuessCount();

	if (Count % Step != 0)
	{
		Subsystem->OnGuessProcessChanged.Broadcast(GS->GetRoundNumber(), Subsystem->GetDefaultLevel() - Count);
	}
}
