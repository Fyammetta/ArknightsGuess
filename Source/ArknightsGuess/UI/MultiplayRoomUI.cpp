// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayRoomUI.h"

#include "ArknightsGuess.h"
#include "PlayerIconInterface.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/DefaultGameStateBase.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

namespace 
{
	constexpr float DefaultTileSize = 1200;
	constexpr int32 MinSize = 4;
}

void UMultiplayRoomUI::NativeConstruct()
{
	Super::NativeConstruct();

		UWorld* World = GetWorld();
		if (!World) return;
	
		HostCastSwitcher->SetActiveWidgetIndex(GetWorld()->GetNetMode() == NM_ListenServer);

		auto GS = World->GetGameState<ADefaultGameStateBase>();
	
		if (!GS) return;
	
		if (!GS->PlayerArray.IsEmpty())
			OnPlayerJoinedOrLeft(World->GetFirstPlayerController()->GetPlayerState<APlayerState>(),true);
		GS->OnMultiplayerNumChanged.AddUniqueDynamic(this,  &UMultiplayRoomUI::OnPlayerJoinedOrLeft);
	

	
}

bool UMultiplayRoomUI::UpdateSize()
{
	UWorld* World = GetWorld();
	if (!World) return false;
	
	AGameStateBase* GS = World->GetGameState();
	if (!GS) return false;
	int32 NewSize = FMath::Max(MinSize,FMath::Floor(FMath::Pow(GS->PlayerArray.Num(),.5f)));
	if (Size != NewSize)
	{
		
		Size = NewSize;
		return true;
	}
	return false;
}

float UMultiplayRoomUI::GetDesiredSize() const
{
	return DefaultTileSize / Size;
}

void UMultiplayRoomUI::OnPlayerJoinedOrLeft(APlayerState* Player, bool bWasJoined)
{
	
	UWorld* World = GetWorld();
	if (!World) return;
	AGameStateBase* GS = World->GetGameState();
	if (!GS) return;
	
	TArray<UObject*> Objects {};
	for (auto PS : GS->PlayerArray)
	{
		if (PS->Implements<UPlayerIconInterface>())
		{
			Objects.Add(PS);
		}
	}
	UE_LOG(LogArknights,Log, TEXT("[UMultiplayRoomUI::NativeConstruct] Set up player list for %d players"),Objects.Num())
	PlayerList->SetListItems(Objects);
	UpdateSize();
	PlayerList->SetEntryHeight(GetDesiredSize());
	PlayerList->SetEntryWidth(GetDesiredSize());
}
