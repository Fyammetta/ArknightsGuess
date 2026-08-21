// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayRoomUI.h"
#include "ArknightsGuess.h"
#include "DevNotificationSubsystem.h"
#include "PlayerIconInterface.h"
#include "SocketSubsystem.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/DefaultGameStateBase.h"
#include "Core/DefaultPlayerController.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GuessGame/GuessGameSettings.h"
#include "Operators/OperatorFunctionLibrary.h"
#include "Operators/OperatorSubsystem.h"
#include "Operators/OperatorTags.h"


namespace 
{
	constexpr float DefaultTileSize = 1200;
	constexpr int32 MinSize = 4;
}

void UMultiplayRoomUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	auto World = GetWorld();
	if (!World) return;
	
	HostCastSwitcher->SetActiveWidgetIndex(World->GetNetMode() == NM_ListenServer);
	
	bool bCanBind = false;
	TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);
	auto GS = World->GetGameState<ADefaultGameStateBase>();

	if (!GS) return;
	OnPlayerJoinedOrLeft();
	GS->OnMultiplayerNumChanged.AddUniqueDynamic(this,  &UMultiplayRoomUI::OnPlayerJoinedOrLeft);
	
	if (LocalIp->IsValid())
	{
		IP_DisplayText->SetText(FText::FromString(LocalIp->ToString(false)));
	}
	if (auto Driver = World->GetNetDriver())
		if (auto Addr = Driver->GetLocalAddr())
			PORT_DisplayText->SetText(FText::FromString(FString::FromInt(Addr->GetPort())));

	if (auto* GameState = World ? GetWorld()->GetGameState() : nullptr)
	{
		if (auto* DS = Cast<ADefaultGameStateBase>(GameState))
		{
			DS->OnMultiplayerNumChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnPlayerJoinedOrLeft);
		}
	}

	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UOperatorSubsystem>())
	{
		Subsystem->OnGameSettingChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnGameSettingChanged);
	}

	UpdateSize();
}

void UMultiplayRoomUI::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (auto* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		if (auto* DS = Cast<ADefaultGameStateBase>(GameState))
		{
			DS->OnMultiplayerNumChanged.RemoveDynamic(this, &UMultiplayRoomUI::OnPlayerJoinedOrLeft);
		}
	}

	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UOperatorSubsystem>())
	{
		Subsystem->OnGameSettingChanged.RemoveDynamic(this, &UMultiplayRoomUI::OnGameSettingChanged);
	}
}

void UMultiplayRoomUI::BindButtons()
{
	Super::BindButtons();

	if (PrepareButton)
		PrepareButton->OnClicked.AddUniqueDynamic(this, &UMultiplayRoomUI::OnPrepareClicked);
	if (QuitGameButton)
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &UMultiplayRoomUI::OnQuitRoomClicked);
}

FReply UMultiplayRoomUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Sample image click handling is implemented in the base class.
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMultiplayRoomUI::OnStartGameClicked()
{
	UE_LOG(LogArknights, Log, TEXT("[MultiplayRoomUI] OnStartGameClicked | Mode=%s"), *GameMode.ToString());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[MultiplayRoomUI] OnStartGameClicked failed: no World")); return; }

	if (GameMode != GameModeTags::Mosaic())
	{
		if (auto Subsystem = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
			Subsystem->ShowNotificationTemplate(EDevNotificationTemplate::NotImplemented);
		return;
	}
	
	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr)
	{
		if (PC->IsAllPlayerReady())
			PC->StartGame(GameMode);
		else
		{
			DEV_ONSCREEN_TIPS(TEXT("There're players are not ready for start!"));
		}
	}

}

void UMultiplayRoomUI::OnLevelSliderChanged(float Value)
{
	Super::OnLevelSliderChanged(Value);
	BroadcastSetting(SettingTags::DefaultLevel(), static_cast<int32>(Value));
}

void UMultiplayRoomUI::OnGuessCountSliderChanged(float Value)
{
	Super::OnGuessCountSliderChanged(Value);
	BroadcastSetting(SettingTags::MaxGuessCount(), static_cast<int32>(Value));
}

void UMultiplayRoomUI::OnShuffleLimitSliderChanged(float Value)
{
	Super::OnShuffleLimitSliderChanged(Value);
	BroadcastSetting(SettingTags::ShuffleLimit(), static_cast<int32>(Value));
}

void UMultiplayRoomUI::OnHintFreqSliderChanged(float Value)
{
	Super::OnHintFreqSliderChanged(Value);
	BroadcastSetting(SettingTags::HintFrequency(), static_cast<int32>(Value));
}

void UMultiplayRoomUI::OnMosaicModeClicked()
{
	Super::OnMosaicModeClicked();
	if (auto PC = GetWorld()->GetFirstPlayerController<ADefaultPlayerController>())
		PC->Server_UpdateGameSetting(GameModeTags::Mosaic(), GameMode.ToString());
}

void UMultiplayRoomUI::OnPartModeClicked()
{
	Super::OnPartModeClicked();
	if (auto PC = GetWorld()->GetFirstPlayerController<ADefaultPlayerController>())
		PC->Server_UpdateGameSetting(GameModeTags::Part(), GameMode.ToString());
}

void UMultiplayRoomUI::OnQuitRoomClicked()
{
	if (auto PC = GetWorld()->GetFirstPlayerController<ADefaultPlayerController>())
	{
		PC->QuitServer();
	}

}

void UMultiplayRoomUI::OnPrepareClicked()
{
	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr)
	{
		PC->PreparedForStart();
	}
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
	PlayerList->SetEntryHeight(GetDesiredHeight());
	PlayerList->SetEntryWidth(GetDesiredHeight());
}

void UMultiplayRoomUI::OnGameSettingChanged(FGameplayTag SettingTag, const FString& NewValue)
{
	if (SettingTag == SettingTags::DefaultLevel())
	{
		const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
		Text_DefaultLevel->SetText(FText::AsNumber(FMath::DivideAndRoundUp(FCString::Atoi(*NewValue), Step)));
	}
	else if (SettingTag == SettingTags::ShuffleLimit())
	{
		Text_ShuffleLimit->SetText(FText::FromString(NewValue));
	}
	else if (SettingTag == SettingTags::MaxGuessCount())
	{
		Text_GuessCount->SetText(FText::FromString(NewValue));
	}
	else if (SettingTag == SettingTags::HintFrequency())
	{
		Text_HintFrequency->SetText(FText::FromString(NewValue));
	}
	else if (SettingTag.MatchesTag(GameModeTags::Root()))
	{
		// Gamemode switched: keep local display & sample in sync with the new mode
		if (GameModeTextMapping.Contains(SettingTag))
			Text_GameMode->SetText(GameModeTextMapping[SettingTag]);

		GameMode = SettingTag;
		if (GameMode == GameModeTags::Mosaic())
		{
			ExchangeButtonStyle();
			RefreshSampleClarity();
		}
		else if (GameMode == GameModeTags::Part())
		{
			ExchangeButtonStyle();
			UOperatorFunctionLibrary::SetOperatorDisplayPart(SampleImage->GetDynamicMaterial(), FVector(0, 0, 1));
		}
	}
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

float UMultiplayRoomUI::GetDesiredHeight() const
{
	return DefaultTileSize / Size;
}

void UMultiplayRoomUI::BroadcastSetting(const FGameplayTag& SettingTag, int32 Value)
{
	if (auto PC = GetWorld()->GetFirstPlayerController<ADefaultPlayerController>())
	{
		PC->Server_UpdateGameSetting(SettingTag, FString::FromInt(Value));
	}
}
