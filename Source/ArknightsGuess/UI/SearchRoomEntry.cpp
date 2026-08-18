// Fill out your copyright notice in the Description page of Project Settings.


#include "SearchRoomEntry.h"

#include "ArknightsGuess.h"
#include "ArknightsGuess/Core/DefaultPlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void USearchRoomEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Select)
		Button_Select->OnClicked.AddUniqueDynamic(this, &USearchRoomEntry::OnSelectClicked);
}

void USearchRoomEntry::NativeDestruct()
{
	if (Button_Select)
		Button_Select->OnClicked.RemoveDynamic(this, &USearchRoomEntry::OnSelectClicked);

	Super::NativeDestruct();
}

void USearchRoomEntry::InitEntry(const FOnlineSessionSearchResult& InResult)
{
	Target = InResult;

	if (Text_RoomName)
	{
		FString RoomName;
		if (!Target.Session.SessionSettings.Get(TEXT("RoomName"), RoomName) || RoomName.IsEmpty())
			RoomName = Target.Session.OwningUserName;
		Text_RoomName->SetText(FText::FromString(RoomName));
	}

	if (Text_PlayerNum)
	{
		const int32 MaxPlayers = Target.Session.SessionSettings.NumPublicConnections;
		const int32 CurrentPlayers = FMath::Max(MaxPlayers - Target.Session.NumOpenPublicConnections, 0);
		Text_PlayerNum->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentPlayers, MaxPlayers)));
	}

	if (Image_Ping)
	{
		auto Material = Image_Ping->GetDynamicMaterial();
		Material->SetScalarParameterValue(FName("Ping"), Target.PingInMs);
	}
}

void USearchRoomEntry::OnSelectClicked()
{
	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr)
	{
		PC->JoinServer(Target);
	}
}
