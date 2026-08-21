// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayRoomUI.h"

#include "ArknightsGuess.h"
#include "PlayerIconInterface.h"
#include "SocketSubsystem.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Slider.h"
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
#include "Operators/OperatorTypes.h"
#include "Operators/OperatorUISettings.h"

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
	

	OnPlayerJoinedOrLeft();
	GS->OnMultiplayerNumChanged.AddUniqueDynamic(this,  &UMultiplayRoomUI::OnPlayerJoinedOrLeft);
	
	bool bCanBind = false;
	TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);
	
	if (LocalIp->IsValid())
	{
		IP_DisplayText->SetText(FText::FromString(LocalIp->ToString(false)));
	}
	if (auto Driver = World->GetNetDriver())
		if (auto Addr = Driver->GetLocalAddr())
			PORT_DisplayText->SetText(FText::FromString(FString::FromInt(Addr->GetPort())));
	
	if (!GameMode.IsValid())
	{
		GameMode = GameModeTags::Mosaic();
	}
	
	BindSliders();
	BindButtons();
	
	// Cast: receives settings broadcast from Host
	if (auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnGameSettingChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnGameSettingChanged);
	}
	
	// Init slider values from rule settings
	if (auto* RuleSettings = UGuessGameSettings::Get())
	{
		LevelSlider->SetValue(static_cast<float>(RuleSettings->DefaultLevel));
		LevelSlider->SetStepSize(static_cast<float>(RuleSettings->ClarityPerLevel));
		LevelValueText->SetText(FText::AsNumber(RuleSettings->DefaultLevel / RuleSettings->ClarityPerLevel));
		
		GuessCountSlider->SetValue(static_cast<float>(RuleSettings->MaxGuessCount));
		GuessCountValueText->SetText(FText::AsNumber(RuleSettings->MaxGuessCount));
		
		HintFreqSlider->SetValue(static_cast<float>(RuleSettings->HintFrequency));
		HintFreqValueText->SetText(FText::AsNumber(RuleSettings->HintFrequency));
		
		const float ShuffleValue = FMath::Min(RuleSettings->ShuffleLimit, ShuffleLimitSlider->GetMaxValue());
		ShuffleLimitSlider->SetValue(ShuffleValue);
		ShuffleLimitValueText->SetText(FText::AsNumber(ShuffleValue));
	}
	
	// Sample image (mirror of UGameMainUI)
	if (auto* UISettings = UOperatorUISettings::Get())
	{
		if (SampleImage && UISettings->SampleTex.IsValid())
		{
			SampleImage->GetDynamicMaterial()->SetTextureParameterValue(TEXT("Tex"), UISettings->SampleTex.LoadSynchronous());
		}
	}

	RefreshSampleMaterial();
	RefreshSampleClarity();
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

void UMultiplayRoomUI::NativeDestruct()
{
	if (auto* Sub = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Sub->OnGameSettingChanged.RemoveDynamic(this, &UMultiplayRoomUI::OnGameSettingChanged);
	}
	Super::NativeDestruct();
}

FReply UMultiplayRoomUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SampleImage && GameMode == GameModeTags::Mosaic())
	{
		const FGeometry& SampleGeo = SampleImage->GetCachedGeometry();
		if (SampleGeo.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			bShowGameplayLevel = !bShowGameplayLevel;
			const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
			LevelSlider->SetStepSize(bShowGameplayLevel ? static_cast<float>(Step) : 1.0f);
			if (bShowGameplayLevel)
			{
				const int32 Rounded = FMath::DivideAndRoundUp(static_cast<int32>(LevelSlider->GetValue()), Step) * Step;
				LevelSlider->SetValue(static_cast<float>(Rounded));
			}
			RefreshSampleClarity();
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void UMultiplayRoomUI::OnLevelSliderChanged(float Value)
{
	RefreshSampleClarity();
	BroadcastSetting(SettingTags::DefaultLevel(), static_cast<int32>(Value));
}

void UMultiplayRoomUI::OnGuessCountSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	GuessCountValueText->SetText(FText::AsNumber(IntVal));
	BroadcastSetting(SettingTags::MaxGuessCount(), IntVal);
}

void UMultiplayRoomUI::OnShuffleLimitSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	ShuffleLimitValueText->SetText(FText::AsNumber(IntVal));
	BroadcastSetting(SettingTags::ShuffleLimit(), IntVal);
}

void UMultiplayRoomUI::OnHintFreqSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	HintFreqValueText->SetText(FText::AsNumber(IntVal));
	BroadcastSetting(SettingTags::HintFrequency(), IntVal);
}

void UMultiplayRoomUI::OnStartGameClicked()
{
	// Bind only: implementation intentionally left empty.
}

void UMultiplayRoomUI::OnPrepareClicked()
{
	// Bind only: implementation intentionally left empty.
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
		if (GameModeTextMapping.Contains(SettingTag))
			Text_GameMode->SetText(GameModeTextMapping[SettingTag]);
		
	}
}

void UMultiplayRoomUI::BindSliders()
{
	if (LevelSlider)
		LevelSlider->OnValueChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnLevelSliderChanged);
	if (GuessCountSlider)
		GuessCountSlider->OnValueChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnGuessCountSliderChanged);
	if (ShuffleLimitSlider)
	{
		ShuffleLimitSlider->OnValueChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnShuffleLimitSliderChanged);
		if (auto Settings = UOperatorUISettings::Get())
		{
			int32 Max = 0;
			Settings->OperatorDatas.LoadSynchronous()->ForeachRow<FOperatorDataRow>(TEXT(""), [&Max](const FName& RowName, const FOperatorDataRow& Row)
			{
				Max += Row.SkinTextures.Num();
			});
			ShuffleLimitSlider->SetValue(Max);
		}
	}
	if (HintFreqSlider)
		HintFreqSlider->OnValueChanged.AddUniqueDynamic(this, &UMultiplayRoomUI::OnHintFreqSliderChanged);
}

void UMultiplayRoomUI::BindButtons()
{
#pragma region OnClicked
#define ONCLICK(Button, Event)\
	if (Button)\
		Button->OnClicked.AddUniqueDynamic(this, &UMultiplayRoomUI::Event);
#pragma endregion
	
	ONCLICK(StartGameButton, OnStartGameClicked);
	ONCLICK(PrepareButton, OnPrepareClicked);
#undef ONCLICK
}

void UMultiplayRoomUI::RefreshSampleMaterial()
{
	if (auto* Settings = UOperatorUISettings::Get())
	{
		SampleImage->SetBrushFromMaterial(Settings->GetMaterial(GameMode));
	}
}

void UMultiplayRoomUI::RefreshSampleClarity()
{
	if (!LevelSlider || !LevelValueText || !SampleImage) return;

	const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
	const int32 RawValue = static_cast<int32>(LevelSlider->GetValue());
	if (bShowGameplayLevel)
	{
		const int32 GameplayValue = FMath::DivideAndRoundUp(RawValue, Step) * Step;
		SampleImage->GetDynamicMaterial()->SetScalarParameterValue(TEXT("Clarity"), GameplayValue);
		LevelValueText->SetText(FText::AsNumber(GameplayValue / Step));
	}
	else
	{
		SampleImage->GetDynamicMaterial()->SetScalarParameterValue(TEXT("Clarity"), RawValue);
		LevelValueText->SetText(FText::AsNumber(RawValue / Step));
	}
}

void UMultiplayRoomUI::BroadcastSetting(const FGameplayTag& SettingTag, int32 Value)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (auto PC = World->GetFirstPlayerController<ADefaultPlayerController>())
	{
		PC->Server_UpdateGameSetting(SettingTag, FString::FromInt(Value));
	}
}
