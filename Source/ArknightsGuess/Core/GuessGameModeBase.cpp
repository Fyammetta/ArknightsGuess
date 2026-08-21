// Fill out your copyright notice in the Description page of Project Settings.

#include "GuessGameModeBase.h"
#include "GuessGameStateBase.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess.h"
#include "GameFramework/PlayerState.h"
#include "Online.h"
#include "Kismet/GameplayStatics.h"


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

void AGuessGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (auto System = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		if (System->IsGameRunning())
		{
			if (auto GS = GetGameState<ADefaultGameStateBase>())
			{
				GS->WhenPlayerOnReady.AddDynamic(this, &AGuessGameModeBase::OnAllReadyForNextRound);
			}
		}
	}
}

void AGuessGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


	if (auto GS = GetGameState<ADefaultGameStateBase>())
	{
		GS->WhenPlayerOnReady.RemoveAll(this);
	}
		
	
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


void AGuessGameModeBase::SetPlayerPrepared(APlayerController* Player)
{
	if (!ReadyPlayers.Contains(Player))
	{
		ReadyPlayers.Add(Player);
		if (auto* GS = GetGameState<ADefaultGameStateBase>())
			GS->NetMulticast_BroadcastOnPlayerReady(Player->GetPlayerState<APlayerState>(), true);

	}
}

void AGuessGameModeBase::SetPlayerUnprepared(APlayerController* Player)
{
	if (ReadyPlayers.Contains(Player))
	{
		ReadyPlayers.Remove(Player);
		if (auto* GS = GetGameState<ADefaultGameStateBase>())
			GS->NetMulticast_BroadcastOnPlayerReady(Player->GetPlayerState<APlayerState>(), false);

	}
}

void AGuessGameModeBase::ResetPreparedPlayers()
{
	
	auto Set = MoveTemp(ReadyPlayers);
	if (auto* GS = GetGameState<ADefaultGameStateBase>())
	{
		for (APlayerController* Player : Set)
		{
			GS->NetMulticast_BroadcastOnPlayerReady(Player->GetPlayerState<APlayerState>(), false);
		}
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


void AGuessGameModeBase::OnAllReadyForNextRound(APlayerState*, bool Ready)
{
	if (!Ready) return;
	if (auto GS = GetGameState<AGameStateBase>())
	{
		if (GS->PlayerArray.Num() == GetReadyPlayerCount())
		{
			StartNewRound();
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&AGuessGameModeBase::ResetPreparedPlayers));
		}
	}
}

void AGuessGameModeBase::ProcessGuess(APlayerController* Player, const FName& OperatorName)
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
			GS->NetMulticast_OnPlayerAnswered(Player->GetPlayerState<APlayerState>(), OperatorName);
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