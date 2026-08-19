// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessPlayerState.h"

#include "ArknightsGuess.h"
#include "Net/UnrealNetwork.h"

void AGuessPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::BeginPlay] NetMode = %d"), GetWorld()->GetNetMode());

	// 注意:不在 BeginPlay 里读取本机设置。服务器会为每个客户端 spawn PlayerState,
	// 若在这里读 UGuessGamerSettings,会把房主的名字/头像填给所有玩家。
	// 名字/头像由每个玩家自己的 PlayerController 上报(见 Server_SetPlayerInfo)。
}

void AGuessPlayerState::OnRep_PlayerIcon()
{
	OnPlayerIconChanged.Broadcast();
}

void AGuessPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGuessPlayerState, PlayerIcon);
}

UTexture2D* AGuessPlayerState::GetPlayerIcon() const
{
	return Cast<UTexture2D>(PlayerIcon.TryLoad());
}

void AGuessPlayerState::ChangePlayerIcon(UTexture2D* Icon)
{
	if (HasAuthority() && Icon)
		PlayerIcon = FSoftObjectPath(Icon);
}

void AGuessPlayerState::ChangePlayerIcon(const FSoftObjectPath& IconPath)
{
	if (HasAuthority() && !IconPath.IsNull())
		PlayerIcon = IconPath;
}

void AGuessPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

}

void AGuessPlayerState::SeamlessTravelTo(class APlayerState* NewPlayerState)
{
	Super::SeamlessTravelTo(NewPlayerState);
	if (auto PS = Cast<AGuessPlayerState>(NewPlayerState))
	{
		PS->ChangePlayerIcon(PlayerIcon);	
	}
}
