// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessPlayerState.h"

#include "ArknightsGuess.h"
#include "DefaultGameStateBase.h"
#include "GuessGamerSettings.h"
#include "Net/UnrealNetwork.h"
#include "UI/UIManagerSubsystem.h"


void AGuessPlayerState::OnRep_PlayerIcon()
{
	OnPlayerIconChangedDelegate.Broadcast();
}

void AGuessPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::BeginPlay] NetMode = %d"), GetWorld()->GetNetMode());
	if (!HasAuthority())
	{
		if (auto System = UUIManagerSubsystem::Get(this))
		{
			System->ShowUI(UITags::Loading());
		}
		OnPlayerIconChangedDelegate.AddUObject(this, &AGuessPlayerState::OnLocalPlayerJoined);
	}
	if (!HasAuthority() || PlayerIcon == nullptr)
	{
		FString Name = UGuessGamerSettings::GetPlayerName();
		UTexture2D* Icon =UGuessGamerSettings::GetPlayerIcon();
		InitPlayerState(Icon->GetPathName(), Name);
		UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::InitPlayerState] Send"));
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
	
	OnPlayerIconChangedDelegate.Broadcast();

}

void AGuessPlayerState::SeamlessTravelTo(class APlayerState* NewPlayerState)
{
	
	Super::SeamlessTravelTo(NewPlayerState);
	if (auto PS = Cast<AGuessPlayerState>(NewPlayerState))
	{
		PS->ChangePlayerIcon(PlayerIcon);	
	}
	OnLocalPlayerJoined();
	UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::SeamlessTravelTo]"))
}

void AGuessPlayerState::OnLocalPlayerJoined()
{
	if (auto System =UUIManagerSubsystem::Get(this))
	{
		System->HideUI(UITags::Loading());
		System->ShowUI(MapTags::MultiRoom());
	}
	
	OnPlayerIconChangedDelegate.RemoveAll(this);
}

void AGuessPlayerState::InitPlayerState_Implementation(const FSoftObjectPath& Icon, const FString& Name)
{
	UE_LOG(LogArknights, Log, TEXT("[AGuessPlayerState::InitPlayerState] Invoke"));

	SetPlayerName(Name);
	ChangePlayerIcon(Cast<UTexture2D>(Icon.TryLoad()));
}
