// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultGameStateBase.h"
#include "ArknightsGuess/GuessGame/GuessGameSettings.h"
#include "ArknightsGuess/GuessGame/UGuessComponentInterface.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "ArknightsGuess.h"

ADefaultGameStateBase::ADefaultGameStateBase()
{
}

void ADefaultGameStateBase::BeginPlay()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] BeginPlay | Authority=%d"), HasAuthority());
	Super::BeginPlay();
	GenerateGameplayComponent();
}

void ADefaultGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADefaultGameStateBase, RoundState);
	DOREPLIFETIME(ADefaultGameStateBase, GuessCount);
	DOREPLIFETIME(ADefaultGameStateBase, RoundNumber);
	DOREPLIFETIME(ADefaultGameStateBase, TriedAnswers);
}

// ---- Game lifecycle RPCs ----

void ADefaultGameStateBase::NetMulticast_StartGame_Implementation(const FGameplayTag& Mode)
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] NetMulticast_StartGame | Mode=%s | Authority=%d"), *Mode.ToString(), HasAuthority());
	if (HasAuthority()) return;

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->StartUp(Mode);
	}
}
// ---- Round state ----

EGuessRoundState ADefaultGameStateBase::GetGuessRoundState() const
{
	return RoundState;
}

void ADefaultGameStateBase::SetGuessRoundState(EGuessRoundState State)
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] SetGuessRoundState | State=%d | Authority=%d"), static_cast<int32>(State), HasAuthority());
	RoundState = State;

	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.Broadcast(State);
	}
}

int32 ADefaultGameStateBase::GetGuessCount() const
{
	return GuessCount;
}

void ADefaultGameStateBase::OnRep_OnGuessStateChanged()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] OnRep_GuessStateChanged | State=%d"), static_cast<int32>(RoundState));
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.Broadcast(RoundState);
	}
}

void ADefaultGameStateBase::OnRep_NextRound()
{
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] OnRep_NextRound | Round=%d"), RoundNumber);
	if (auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessProcessChanged.Broadcast(RoundNumber, Subsystem->GetDefaultLevel());
	}
}

void ADefaultGameStateBase::GenerateGameplayComponent()
{
	auto* Settings = UGuessGameSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent failed: no GuessGameSettings"));
		return;
	}

	auto* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem)
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent failed: no OperatorSubsystem"));
		return;
	}

	const FGameplayTag Mode = Subsystem->GetGameplayMode();
	if (!Mode.IsValid())
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent failed: no GameMode selected"));
		return;
	}

	if (!Settings->ModeComponents.Contains(Mode))
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent: no component registered for mode '%s'"), *Mode.ToString());
		return;
	}

	const TSubclassOf<UActorComponent> Class = Settings->ModeComponents[Mode];
	if (!Class)
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent: null class for mode '%s'"), *Class->GetName());
		return;
	}

	UActorComponent* Comp = NewObject<UActorComponent>(this, Class, TEXT("GameModeComponent"));
	if (!Comp)
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent: NewObject failed for class '%s'"), *Class->GetName());
		return;
	}

	Comp->RegisterComponent();

	if (!Comp->Implements<UUGuessComponentInterface>())
	{
		UE_LOG(LogArknights, Warning, TEXT("[DefaultGS] GenerateGameplayComponent: class '%s' does not implement IUGuessComponentInterface"), *Class->GetName());
		return;
	}

	GuessComponent = Cast<IUGuessComponentInterface>(Comp);
	UE_LOG(LogArknights, Log, TEXT("[DefaultGS] GenerateGameplayComponent | Mode=%s | Component=%s"), *Mode.ToString(), *Class->GetName());
}