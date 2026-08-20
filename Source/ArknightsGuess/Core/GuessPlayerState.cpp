// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessPlayerState.h"

#include "ArknightsGuess.h"
#include "GuessGamerSettings.h"
#include "Net/UnrealNetwork.h"

void AGuessPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::BeginPlay] NetMode = %d"), GetWorld()->GetNetMode());

	if (!PlayerIcon)
	{
		FString Name = UGuessGamerSettings::GetPlayerName();
		UTexture2D* Icon =UGuessGamerSettings::GetPlayerIcon();
		SetPlayerName(Name);
		ChangePlayerIcon(Icon);
	}
}

void AGuessPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGuessPlayerState, PlayerIcon);
}

UTexture2D* AGuessPlayerState::GetPlayerIcon() const
{
	return PlayerIcon;
}

void AGuessPlayerState::ChangePlayerIcon(UTexture2D* Icon)
{
	if (HasAuthority() && Icon)
		PlayerIcon = Icon;
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
