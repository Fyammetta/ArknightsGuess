// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayRoomUI.h"

#include "ArknightsGuess.h"
#include "PlayerIconInterface.h"
#include "SocketSubsystem.h"
#include "Core/LanDiscoverySubsystem.h"
#include "Core/GuessPlayerState.h"
#include "Components/TextBlock.h"
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

	// 方案3:枚举本机全部 IPv4(安卓含热点接口;GetLocalHostAddr 在热点下会拿到移动数据接口,PC 不可达)
	TArray<FString> DisplayIps = ULanDiscoverySubsystem::GetAllLocalIPv4();
	if (!DisplayIps.IsEmpty())
	{
		IP_DisplayText->SetText(FText::FromString(FString::Join(DisplayIps, TEXT("\n"))));
		UE_LOG(LogArknights, Log, TEXT("[MultiplayRoomUI] Local IPs: %s"), *FString::Join(DisplayIps, TEXT(", ")));
	}
	else
	{
		// 兜底:退回 GetLocalHostAddr
		bool bCanBind = false;
		TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);
		if (LocalIp->IsValid())
		{
			IP_DisplayText->SetText(FText::FromString(LocalIp->ToString(false)));
		}
	}
	PORT_DisplayText->SetText(FText::FromString(FString::FromInt(World->URL.Port)));
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
		if (auto* GPS = Cast<AGuessPlayerState>(PS))
		{
			// 绑定头像复制回调,头像到达后刷新列表(AddUniqueDynamic 防重复绑定)
			GPS->OnPlayerIconChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnAnyPlayerIconChanged);
			Objects.Add(GPS);
		}
		else if (PS->Implements<UPlayerIconInterface>())
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

void UMultiplayRoomUI::OnAnyPlayerIconChanged()
{
	// 头像复制可能晚于首次列表渲染,收到变更后重建列表
	OnPlayerJoinedOrLeft(nullptr, true);
}
