// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameModeBase.h"
#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess.h"
#include "Online.h"


void AGuessGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogArknights, Log, TEXT("[GM] PostLogin | Player=%s"), NewPlayer ? *NewPlayer->GetName() : TEXT("null"));

	if (!NewPlayer)
		return;
	

	
	IOnlineSessionPtr SessionPtr = Online::GetSessionInterface();
	if (auto Player = NewPlayer->GetLocalPlayer())
	{
		auto NetId = NewPlayer->GetLocalPlayer()->GetPreferredUniqueNetId();
		if (SessionPtr.IsValid() && NetId.IsValid())
		{
			SessionPtr->RegisterPlayer(NAME_GameSession, *NetId, false);
		}
	}

}

void AGuessGameModeBase::Logout(AController* Exiting)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] Logout | Controller=%s"), Exiting ? *Exiting->GetName() : TEXT("null"));
	Super::Logout(Exiting);
}


void AGuessGameModeBase::EndGame()
{
	UE_LOG(LogArknights, Log, TEXT("[GM] EndGame"));
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->NetMulticast_EndGame();
	}
	else
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] EndGame failed: no GameState"));
		return;
	}

	SetRoundState(EGuessRoundState::WaitingForPlayers);
}

void AGuessGameModeBase::TryStartNewRound(APlayerController* Player)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] TryStartNewRound | Player=%s"), Player ? *Player->GetName() : TEXT("null"));
	auto* GS = GetGameState<AGuessGameStateBase>();
	if (!GS || !Player)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] TryStartNewRound failed: no GameState or Player"));
		return;
	}
	if (ReadyPlayers.Contains(Player))
	{
		UE_LOG(LogArknights, Log, TEXT("[GM] TryStartNewRound: Player already ready, waiting for others | Ready=%d/%d"), ReadyPlayers.Num(), GS->PlayerArray.Num());
		return;
	}

	ReadyPlayers.Add(Player);

	GS->NetMulticast_BroadcastOnPlayerReady(Player, true, FGameplayTag::EmptyTag);
	UE_LOG(LogArknights, Log, TEXT("[GM] TryStartNewRound: Player ready | Ready=%d/%d"), ReadyPlayers.Num(),GS->PlayerArray.Num());
	if (ReadyPlayers.Num() >= GS->PlayerArray.Num())
	{
		UE_LOG(LogArknights, Log, TEXT("[GM] TryStartNewRound: all players ready, starting new round"));
		ReadyPlayers.Empty();
		StartNewRound();
	}
}

void AGuessGameModeBase::StartNewRound()
{
	UE_LOG(LogArknights, Log, TEXT("[GM] StartNewRound"));
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!GS || !Subsystem)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] StartNewRound failed: no GameState or Subsystem"));
		return;
	}

	// Guard: don't start a new round if one is already in progress
	// (e.g. multiple clients calling RequestNextRound simultaneously)
	if (GS->GetGuessRoundState() == EGuessRoundState::Guessing)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] StartNewRound ignored: round already in progress"));
		return;
	}


	// 1. Pick an answer
	CorrectAnswer = Subsystem->GetRandomOperatorData();

	// 2. Push operator data to GameState
	GS->EnterNewRound(CorrectAnswer);


	// 3. Transition to Guessing
	SetRoundState(EGuessRoundState::Guessing);
}

void AGuessGameModeBase::ProcessGuess(const FName& OperatorName)
{
	UE_LOG(LogArknights, Log, TEXT("[GM] ProcessGuess | Answer=%s"), *OperatorName.ToString());
	AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>();
	UOperatorSubsystem* Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);

	if (!Subsystem || !GS || GS->GetGuessRoundState() != EGuessRoundState::Guessing)
	{
		UE_LOG(LogArknights, Warning, TEXT("[GM] ProcessGuess failed: bad state or missing Subsystem/GS"));
		return;
	}

	if (OperatorName == CorrectAnswer.Name.RealName)
	{
		SetRoundState(EGuessRoundState::Verify);
	}
	else
	{
		// else: wrong guess — client gets feedback, clarity level increases on GameState
		int32 Max = Subsystem->GetMaxGuessCount();
		int32 Cur = GS->GetGuessCount();
		if (Cur >= Max)
		{
			SetRoundState(EGuessRoundState::Reveal);
		}
		else
		{
			GS->Clarify();
		}
	}
}

int32 AGuessGameModeBase::GetReadyPlayerCount() const
{
	return ReadyPlayers.Num();
}

void AGuessGameModeBase::SetRoundState(EGuessRoundState NewState) const
{
	UE_LOG(LogArknights, Log, TEXT("[GM] SetRoundState | State=%d"), static_cast<int32>(NewState));
	if (AGuessGameStateBase* GS = GetGameState<AGuessGameStateBase>())
	{
		GS->SetGuessRoundState(NewState);
	}
}