// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMainUI.h"
#include "ArknightsGuess/Core/GuesserPlayerController.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "ArknightsGuess/Operators/OperatorUISettings.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "DevNotification/Public/DevNotificationSettings.h"
#include "DevNotification/Public/DevNotificationSubsystem.h"


void UGameMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	BindSliders();
	BindButtons();

	// Init slider values from settings
	if (auto* Settings = UOperatorUISettings::Get())
	{
		LevelSlider->SetValue(static_cast<float>(Settings->DefaultLevel));
		LevelValueText->SetText(FText::AsNumber(Settings->DefaultLevel));

		GuessCountSlider->SetValue(static_cast<float>(Settings->MaxGuessCount));
		GuessCountValueText->SetText(FText::AsNumber(Settings->MaxGuessCount));

		HintFreqSlider->SetValue(static_cast<float>(Settings->HintFrequency));
		HintFreqValueText->SetText(FText::AsNumber(Settings->HintFrequency));

		if (SampleImage && Settings->SampleTex.IsValid())
		{
			SampleImage->GetDynamicMaterial()->SetTextureParameterValue(TEXT("Tex"),Settings->SampleTex.LoadSynchronous());
		}
	}
}

FReply UGameMainUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bExpandedSettings || SettingsBorder) return FReply::Unhandled();
	
	const FGeometry& Geo = SettingsBorder->GetCachedGeometry();

	if (!Geo.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
	{
		PlayAnimationReverse(ShowSettingsWidget);
		bExpandedSettings = false;
		FReply::Handled();
	}
	
	return FReply::Unhandled();
}

void UGameMainUI::BindSliders()
{
	if (LevelSlider)
		LevelSlider->OnValueChanged.AddDynamic(this, &UGameMainUI::OnLevelSliderChanged);
	if (GuessCountSlider)
		GuessCountSlider->OnValueChanged.AddDynamic(this, &UGameMainUI::OnGuessCountSliderChanged);
	if (ShuffleLimitSlider)
	{
		ShuffleLimitSlider->OnValueChanged.AddDynamic(this, &UGameMainUI::OnShuffleLimitSliderChanged);
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
		HintFreqSlider->OnValueChanged.AddDynamic(this, &UGameMainUI::OnHintFreqSliderChanged);
}

void UGameMainUI::BindButtons()
{
	if (SoloPlayButton)
		SoloPlayButton->OnClicked.AddDynamic(this, &UGameMainUI::OnSoloPlayClicked);
	if (MultiPlayButton)
		MultiPlayButton->OnClicked.AddDynamic(this, &UGameMainUI::OnMultiPlayClicked);
	if (MosaicModeButton)
		MosaicModeButton->OnClicked.AddDynamic(this, &UGameMainUI::OnMosaicModeClicked);
	if (PartModeButton)
		PartModeButton->OnClicked.AddDynamic(this, &UGameMainUI::OnPartModeClicked);
	if (StartGameButton)
		StartGameButton->OnClicked.AddDynamic(this, &UGameMainUI::OnStartGameClicked);
}

// ---- Slider callbacks ----

void UGameMainUI::OnLevelSliderChanged(float Value)
{
	int32 IntVal = Value;
	LevelValueText->SetText(FText::AsNumber(IntVal));
}

void UGameMainUI::OnGuessCountSliderChanged(float Value)
{
	int32 IntVal = Value;
	GuessCountValueText->SetText(FText::AsNumber(IntVal));
}

void UGameMainUI::OnShuffleLimitSliderChanged(float Value)
{
	int32 IntVal = Value;
	ShuffleLimitValueText->SetText(FText::AsNumber(IntVal));
}

void UGameMainUI::OnHintFreqSliderChanged(float Value)
{
	int32 IntVal = Value;
	HintFreqValueText->SetText(FText::AsNumber(IntVal));
}

// ---- Button callbacks ----

void UGameMainUI::OnSoloPlayClicked()
{
	SubWidgetSwitcher->SetActiveWidgetIndex(0);
	PlayAnimationForward(ShowSettingsWidget);
	bExpandedSettings = true;
}

void UGameMainUI::OnMultiPlayClicked()
{
	if (auto Subsystem = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
		Subsystem->ShowNotificationTemplate(EDevNotificationTemplate::UnderDevelopment);
}

void UGameMainUI::OnMosaicModeClicked()
{
	GameMode = TEXT("Mosaic");
}

void UGameMainUI::OnPartModeClicked()
{
	GameMode = TEXT("Part");
}

void UGameMainUI::OnStartGameClicked()
{
	if (!GetWorld()) return;
	if (auto PC = GetWorld()->GetFirstPlayerController<AGuesserPlayerController>())
	{
		PC->StartGame(GameMode);
	}
}
