// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMainUI.h"

#include "Animation/UMGSequencePlayer.h"
#include "ArknightsGuess/Core/DefaultPlayerController.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorTags.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess/Operators/OperatorUISettings.h"
#include "ArknightsGuess/GuessGame/GuessGameSettings.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "DevNotification/Public/DevNotificationSubsystem.h"
#include "ArknightsGuess.h"
#include "OnlineSessionSettings.h"
#include "SearchRoomEntry.h"
#include "UIManagerSubsystem.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (PartSelector)
		PartSelector->OnSelectionChanged.AddUniqueDynamic(this, &UGameMainUI::OnPartSelectionChanged);
}

void UGameMainUI::BindButtons()
{
	Super::BindButtons();

#define ONCLICK(Button, Event) \
	if (Button) \
		Button->OnClicked.AddUniqueDynamic(this, &UGameMainUI::Event);

	ONCLICK(SoloPlayButton, OnSoloPlayClicked);
	ONCLICK(CreateRoomButton, OnCreateRoomClicked);
	ONCLICK(QuitGameButton, OnQuitGameClicked);
	ONCLICK(SettingsButton, OnSettingsClicked);
	ONCLICK(ConfirmCreateRoomButton, OnMultiCreateClicked);
	ONCLICK(JoinRoomButton, OnJoinRoomClicked);
	ONCLICK(ConfirmJoinRoomButton, OnMultiJoinClicked);
	ONCLICK(SearchRoomButton, OnMultiSearchClicked);
	ONCLICK(CancelRoomButton,OnCreateRoomCanceled);
#undef ONCLICK
}

void UGameMainUI::NativeDestruct()
{
	if (PartSelector)
	{
		PartSelector->OnSelectionChanged.RemoveDynamic(this, &UGameMainUI::OnPartSelectionChanged);
	}

	Super::NativeDestruct();
}

void UGameMainUI::OnStartGameClicked()
{
	UE_LOG(LogArknights, Log, TEXT("[MainUI] OnStartGameClicked | Mode=%s"), *GameMode.ToString());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[MainUI] OnStartGameClicked failed: no World")); return; }

	if (GameMode != GameModeTags::Mosaic())
	{
		if (auto Subsystem = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
			Subsystem->ShowNotificationTemplate(EDevNotificationTemplate::NotImplemented);
		return;
	}

	TWeakObjectPtr<ADefaultPlayerController> PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr;
	PlayAnimation(ShowSettingsWidget,1,1,EUMGSequencePlayMode::Reverse)->OnSequenceFinishedPlaying().AddWeakLambda(this,
		[PC, Mode = GameMode](UUMGSequencePlayer&)
		{
			if (PC.IsValid())
				PC->StartGame(Mode);
		});

}

void UGameMainUI::OnPartModeClicked()
{
	if (GameMode == GameModeTags::Part()) return;
	GameMode = GameModeTags::Part();

	ExchangeButtonStyle();
	FVector Detail = FVector(0,0,1);
	if (!PartDetails.IsEmpty())
	{
		auto Index = PartSelector->GetSelectedIndex();
		Detail = PartDetails.IsValidIndex(Index) ? PartDetails[Index] : PartDetails[FMath::RandRange(0, PartDetails.Num() - 1)];
	}
	UOperatorFunctionLibrary::SetOperatorDisplayPart(SampleImage->GetDynamicMaterial(),Detail);

}

FReply UGameMainUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Result = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (!Result.IsEventHandled())
	{

		if (!bExpandedSettings || !SettingsBorder) return FReply::Unhandled();

		FGeometry Geo{};
		switch (SubWidgetSwitcher->GetActiveWidgetIndex())
		{
		case 0 :	Geo = SettingsBorder->GetCachedGeometry();break;
		case 1 :	Geo = CreateRoomBorder->GetCachedGeometry();break;
		case 2 :	Geo = JoinRoomBorder->GetCachedGeometry();break;
		}


		if (!Geo.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			PlayAnimationReverse(ShowSettingsWidget);
			bExpandedSettings = false;
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}
	return Result;
}

void UGameMainUI::OnPartSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (PartDetails.IsEmpty()) return;

	const int32 Index = PartSelector->GetSelectedIndex();
	const FVector Detail = PartDetails.IsValidIndex(Index) ? PartDetails[Index] : PartDetails[FMath::RandRange(0, PartDetails.Num() - 1)];
	UOperatorFunctionLibrary::SetOperatorDisplayPart(SampleImage->GetDynamicMaterial(), Detail);
}

void UGameMainUI::OnSoloPlayClicked()
{
	SubWidgetSwitcher->SetActiveWidgetIndex(0);
	PlayAnimationForward(ShowSettingsWidget);
	bExpandedSettings = true;
}

void UGameMainUI::OnCreateRoomClicked()
{
	
	SubWidgetSwitcher->SetActiveWidgetIndex(1);
	PlayAnimationForward(ShowSettingsWidget);
	bExpandedSettings = true;
	
}

void UGameMainUI::OnCreateRoomCanceled()
{
	PlayAnimationReverse(ShowSettingsWidget);
	bExpandedSettings = false;
}

void UGameMainUI::OnMultiCreateClicked()
{
	if (!GetWorld()) return;

	TWeakObjectPtr<ADefaultPlayerController> PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr;
	PlayAnimation(ShowSettingsWidget,1,1,EUMGSequencePlayMode::Reverse)->OnSequenceFinishedPlaying().AddWeakLambda(this,
		[PC, Name = RoomNameInputText->GetText().ToString(), Port = PortInputText->GetText().ToString()](UUMGSequencePlayer&)
		{
			if (PC.IsValid())
			{
				if (Name.IsEmpty())
				{
					UDevNotificationSubsystem::Get(PC.Get())->ShowNotification(TEXT("Room name should not be empty!"));
					return;
				}
		
				PC->PrepareForMultiply(Name, Port);
			}
		});
}

void UGameMainUI::OnMultiSearchClicked()
{
	if (!GetWorld()) return;
	if (auto PC = GetWorld()->GetFirstPlayerController<ADefaultPlayerController>())
	{
		FString RoomName = RoomNameInputText->GetText().ToString();
		if (!PC->TryFindLocalServer(FOnFindSessionsCompleteDelegate::CreateUObject(this,&UGameMainUI::OnLocalServerSearchComplete), SearchHandle))
		{
			DEV_ONSCREEN_TIPS(TEXT("Fail to find server!"));
		}
	}
}

void UGameMainUI::OnMultiJoinClicked()
{
	if (!GetWorld()) return;
	
	TWeakObjectPtr<ADefaultPlayerController> PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr;
	PlayAnimation(ShowSettingsWidget,1,1,EUMGSequencePlayMode::Reverse)->OnSequenceFinishedPlaying().AddWeakLambda(this,
		[PC, AddrText = ServerAddressInputText->GetText()](UUMGSequencePlayer&)
		{
			if (PC.IsValid())
			{
				if (AddrText.IsEmpty())
				{
					UDevNotificationSubsystem::Get(PC.Get())->ShowNotification(TEXT("You Should Input The Url You Are Joining!"));
					return;
				}
				PC->JoinServer(AddrText.ToString());
			}
		});
}

void UGameMainUI::OnQuitGameClicked()
{
	UE_LOG(LogArknights, Log, TEXT("[MainUI] OnQuitGameClicked"));
	
	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(),EQuitPreference::Type::Quit,false);
		return;
	}
	UE_LOG(LogArknights, Error, TEXT("[MainUI] Fail to quit game, FORCE QUIT"));
	GEngine->DeferredCommands.Add(TEXT("quit"));

}

void UGameMainUI::OnSettingsClicked()
{
	auto Subsystem = UUIManagerSubsystem::Get(this);
	if (!Subsystem) return;
		
	Subsystem->ShowUI(UITags::Settings());
}

void UGameMainUI::OnJoinRoomClicked()
{
	SubWidgetSwitcher->SetActiveWidgetIndex(2);
	PlayAnimationForward(ShowSettingsWidget);
	bExpandedSettings = true;
}

void UGameMainUI::OnLocalServerSearchComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		DEV_ONSCREEN_TIPS(TEXT("Fail to find server!"));
		return;
	}
	auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<ADefaultPlayerController>() : nullptr;
	if (!PC)
	{
		return;
	}
	if (!SearchRoomEntryClass)
	{
		DEV_ONSCREEN_TIPS(TEXT("SearchRoomEntryClass not set!"));
		return;
	}
	RoomList->ClearChildren();
	UE_LOG(LogArknights, Log, TEXT("[MainUI] Local server search complete, find %d sessions"), PC->GetAllSessions().Num());
	for (const FOnlineSessionSearchResult& Session : PC->GetAllSessions())
	{
		auto* Entry = CreateWidget<USearchRoomEntry>(this, SearchRoomEntryClass);
		if (!Entry) continue;
		Entry->InitEntry(Session);
		RoomList->AddChild(Entry);
	}
}
