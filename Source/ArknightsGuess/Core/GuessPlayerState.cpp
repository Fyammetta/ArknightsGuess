// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessPlayerState.h"

#include "Net/UnrealNetwork.h"

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
