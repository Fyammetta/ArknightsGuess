// Fill out your copyright notice in the Description page of Project Settings.


#include "SearchRoomEntry.h"

#include "ArknightsGuess.h"
#include "ArknightsGuess/Core/DefaultPlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

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

void USearchRoomEntry::InitEntry(const FLanRoomInfo& InRoom)
{
	Target = InRoom;

	if (Text_RoomName)
	{
		Text_RoomName->SetText(FText::FromString(Target.RoomName));
	}

	if (Text_PlayerNum)
	{
		// 自建发现不携带玩家数,这里显示房间的可达地址,便于核对
		FString Addr = Target.BestIP.IsEmpty() ? (Target.IPs.Num() > 0 ? Target.IPs[0] : FString()) : Target.BestIP;
		Text_PlayerNum->SetText(FText::FromString(Addr));
	}

	if (Image_Ping)
	{
		auto Material = Image_Ping->GetDynamicMaterial();
		if (Material)
		{
			Material->SetScalarParameterValue(FName("Ping"), Target.PingMs);
		}
	}
}

void USearchRoomEntry::OnSelectClicked()
{
	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr)
	{
		PC->JoinDiscoveredRoom(Target);
	}
}
