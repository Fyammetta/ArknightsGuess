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
#include "UIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GameMode.IsValid())
	{
		GameMode = GameModeTags::Mosaic();
	}

	BindSliders();
	BindButtons();

	if (PartSelector)
		PartSelector->OnSelectionChanged.AddUniqueDynamic(this, &UGameMainUI::OnPartSelectionChanged);


	// Init slider values from rule settings
	if (auto* RuleSettings = UGuessGameSettings::Get())
	{
		LevelSlider->SetValue(static_cast<float>(RuleSettings->DefaultLevel));
		const float ClarityStep = static_cast<float>(RuleSettings->ClarityPerLevel);
		LevelSlider->SetStepSize(ClarityStep);
		LevelValueText->SetText(FText::AsNumber(RuleSettings->DefaultLevel / RuleSettings->ClarityPerLevel));

		GuessCountSlider->SetValue(static_cast<float>(RuleSettings->MaxGuessCount));
		GuessCountValueText->SetText(FText::AsNumber(RuleSettings->MaxGuessCount));

		HintFreqSlider->SetValue(static_cast<float>(RuleSettings->HintFrequency));
		HintFreqValueText->SetText(FText::AsNumber(RuleSettings->HintFrequency));

		float Value = FMath::Min(RuleSettings->ShuffleLimit, ShuffleLimitSlider->GetMaxValue());
		ShuffleLimitSlider->SetValue(Value);
		ShuffleLimitValueText->SetText(FText::AsNumber(Value));
	}

	// Sample image (UI config)
	if (auto* UISettings = UOperatorUISettings::Get())
	{
		if (SampleImage && UISettings->SampleTex.IsValid())
		{
			SampleImage->GetDynamicMaterial()->SetTextureParameterValue(TEXT("Tex"), UISettings->SampleTex.LoadSynchronous());
		}
	}
}

void UGameMainUI::NativeDestruct()
{
	if (PartSelector)
	{
		PartSelector->OnSelectionChanged.RemoveDynamic(this, &UGameMainUI::OnPartSelectionChanged);
	}

	Super::NativeDestruct();
}

FReply UGameMainUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// ---- Sample image click: toggle gameplay / actual level (Mosaic only) ----
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

	if (!bExpandedSettings || !SettingsBorder) return FReply::Unhandled();

	const FGeometry& Geo = SettingsBorder->GetCachedGeometry();

	if (!Geo.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
	{
		PlayAnimationReverse(ShowSettingsWidget);
		bExpandedSettings = false;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UGameMainUI::BindSliders()
{
	if (LevelSlider)
		LevelSlider->OnValueChanged.AddUniqueDynamic(this, &UGameMainUI::OnLevelSliderChanged);
	if (GuessCountSlider)
		GuessCountSlider->OnValueChanged.AddUniqueDynamic(this, &UGameMainUI::OnGuessCountSliderChanged);
	if (ShuffleLimitSlider)
	{
		ShuffleLimitSlider->OnValueChanged.AddUniqueDynamic(this, &UGameMainUI::OnShuffleLimitSliderChanged);
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
		HintFreqSlider->OnValueChanged.AddUniqueDynamic(this, &UGameMainUI::OnHintFreqSliderChanged);
}

void UGameMainUI::BindButtons()
{
#define ONCLICK(Button,Event)\
	if (Button)\
		Button->OnClicked.AddUniqueDynamic(this, &UGameMainUI::Event);
	ONCLICK(SoloPlayButton,OnSoloPlayClicked);
	ONCLICK(MultiPlayButton,OnMultiPlayClicked);
	ONCLICK(MosaicModeButton,OnMosaicModeClicked);
	ONCLICK(PartModeButton,OnPartModeClicked);
	ONCLICK(StartGameButton,OnStartGameClicked);
	ONCLICK(QuitGameButton,OnQuitGameClicked);
	ONCLICK(SettingsButton,OnSettingsClicked);

}

void UGameMainUI::ExchangeButtonStyle()
{
	auto Style = MosaicModeButton->GetStyle();
	auto Text = MosaicModeButton->GetColorAndOpacity();

	MosaicModeButton->SetStyle(PartModeButton->GetStyle());
	PartModeButton->SetStyle(Style);

	MosaicModeButton->SetColorAndOpacity(PartModeButton->GetColorAndOpacity());
	PartModeButton->SetColorAndOpacity(Text);

	ModeSwitcher->SetActiveWidgetIndex(ModeSwitcher->GetActiveWidgetIndex() == 0 ? 1 : 0);

	RefreshSampleMaterial();
}

void UGameMainUI::RefreshSampleMaterial()
{
	if (auto Settings = UOperatorUISettings::Get())
	{
		if (auto Material = Settings->GetMaterial(GameMode))
		{
			SampleImage->SetBrushFromMaterial(Material);
		}
	}
}

void UGameMainUI::RefreshSampleClarity()
{
	if (!SampleImage || !LevelSlider) return;

	const int32 Step = UGuessGameSettings::Get()->ClarityPerLevel;
	const int32 RawValue = static_cast<int32>(LevelSlider->GetValue());

	if (bShowGameplayLevel)
	{
		// Gameplay level: round up to nearest multiple of ClarityPerLevel
		const int32 GameplayValue = FMath::DivideAndRoundUp(RawValue, Step) * Step;
		LevelValueText->SetText(FText::AsNumber(GameplayValue / Step));
		UOperatorFunctionLibrary::SetOperatorClarity(SampleImage->GetDynamicMaterial(), GameplayValue);
	}
	else
	{
		// Actual level: pass raw value as-is
		LevelValueText->SetText(FText::AsNumber(RawValue / Step));
		UOperatorFunctionLibrary::SetOperatorClarity(SampleImage->GetDynamicMaterial(), RawValue);
	}
}

// ---- Slider callbacks ----

void UGameMainUI::OnLevelSliderChanged(float Value)
{
	RefreshSampleClarity();
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
	
	// SubWidgetSwitcher->SetActiveWidgetIndex(1);
	// PlayAnimationForward(ShowSettingsWidget);
	// bExpandedSettings = true;
	
	GetWorld()->GetFirstPlayerController<ADefaultPlayerController>()->PrepareForMultiply(TEXT("25565"));
}

void UGameMainUI::OnMosaicModeClicked()
{
	if (GameMode == GameModeTags::Mosaic()) return;
	GameMode = GameModeTags::Mosaic();

	ExchangeButtonStyle();
	RefreshSampleClarity();

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

void UGameMainUI::OnPartSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (PartDetails.IsEmpty()) return;

	const int32 Index = PartSelector->GetSelectedIndex();
	const FVector Detail = PartDetails.IsValidIndex(Index) ? PartDetails[Index] : PartDetails[FMath::RandRange(0, PartDetails.Num() - 1)];
	UOperatorFunctionLibrary::SetOperatorDisplayPart(SampleImage->GetDynamicMaterial(), Detail);
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
