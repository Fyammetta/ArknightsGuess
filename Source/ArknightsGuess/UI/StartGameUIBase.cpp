// Fill out your copyright notice in the Description page of Project Settings.


#include "StartGameUIBase.h"

#include "ArknightsGuess.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "DevNotification/Public/DevNotificationSubsystem.h"
#include "GuessGame/GuessGameSettings.h"
#include "Operators/OperatorFunctionLibrary.h"
#include "Operators/OperatorSubsystem.h"
#include "Operators/OperatorTags.h"
#include "Operators/OperatorUISettings.h"

void UStartGameUIBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GameMode.IsValid())
	{
		GameMode = GameModeTags::Mosaic();
	}

	BindSliders();
	BindButtons();

	InitRuleSettings();
	InitSampleImage();

	RefreshSampleMaterial();
	RefreshSampleClarity();
}

void UStartGameUIBase::NativeDestruct()
{
	Super::NativeDestruct();
}

FReply UStartGameUIBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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
	return FReply::Unhandled();
}

void UStartGameUIBase::InitRuleSettings()
{
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

		const float ShuffleValue = FMath::Min(RuleSettings->ShuffleLimit, ShuffleLimitSlider->GetMaxValue());
		ShuffleLimitSlider->SetValue(ShuffleValue);
		ShuffleLimitValueText->SetText(FText::AsNumber(ShuffleValue));
	}
}

void UStartGameUIBase::InitSampleImage()
{
	// Sample image (UI config)
	if (auto* UISettings = UOperatorUISettings::Get())
	{
		if (SampleImage && UISettings->SampleTex.IsValid())
		{
			SampleImage->GetDynamicMaterial()->SetTextureParameterValue(TEXT("Tex"), UISettings->SampleTex.LoadSynchronous());
		}
	}
}

void UStartGameUIBase::BindSliders()
{
	if (LevelSlider)
		LevelSlider->OnValueChanged.AddUniqueDynamic(this, &UStartGameUIBase::OnLevelSliderChanged);
	if (GuessCountSlider)
		GuessCountSlider->OnValueChanged.AddUniqueDynamic(this, &UStartGameUIBase::OnGuessCountSliderChanged);
	if (ShuffleLimitSlider)
	{
		ShuffleLimitSlider->OnValueChanged.AddUniqueDynamic(this, &UStartGameUIBase::OnShuffleLimitSliderChanged);
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
		HintFreqSlider->OnValueChanged.AddUniqueDynamic(this, &UStartGameUIBase::OnHintFreqSliderChanged);
}

void UStartGameUIBase::BindButtons()
{
	if (MosaicModeButton)
		MosaicModeButton->OnClicked.AddUniqueDynamic(this, &UStartGameUIBase::OnMosaicModeClicked);
	if (PartModeButton)
		PartModeButton->OnClicked.AddUniqueDynamic(this, &UStartGameUIBase::OnPartModeClicked);
	if (StartGameButton)
		StartGameButton->OnClicked.AddUniqueDynamic(this, &UStartGameUIBase::OnStartGameClicked);
}

void UStartGameUIBase::OnLevelSliderChanged(float Value)
{
	RefreshSampleClarity();
}

void UStartGameUIBase::OnGuessCountSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	if (GuessCountValueText)
		GuessCountValueText->SetText(FText::AsNumber(IntVal));
}

void UStartGameUIBase::OnShuffleLimitSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	if (ShuffleLimitValueText)
		ShuffleLimitValueText->SetText(FText::AsNumber(IntVal));
}

void UStartGameUIBase::OnHintFreqSliderChanged(float Value)
{
	const int32 IntVal = static_cast<int32>(Value);
	if (HintFreqValueText)
		HintFreqValueText->SetText(FText::AsNumber(IntVal));
}

void UStartGameUIBase::OnMosaicModeClicked()
{
	if (GameMode == GameModeTags::Mosaic()) return;
	GameMode = GameModeTags::Mosaic();

	ExchangeButtonStyle();
	RefreshSampleClarity();
}

void UStartGameUIBase::OnPartModeClicked()
{
	if (GameMode == GameModeTags::Part()) return;
	GameMode = GameModeTags::Part();

	ExchangeButtonStyle();
	UOperatorFunctionLibrary::SetOperatorDisplayPart(SampleImage->GetDynamicMaterial(), FVector(0, 0, 1));
}

void UStartGameUIBase::OnStartGameClicked()
{
	// Default start behaviour. Subclasses override for their own flow.
	UE_LOG(LogArknights, Log, TEXT("[StartGameUIBase] OnStartGameClicked | Mode=%s"), *GameMode.ToString());
	if (!GetWorld()) { UE_LOG(LogArknights, Warning, TEXT("[StartGameUIBase] OnStartGameClicked failed: no World")); return; }

	if (GameMode != GameModeTags::Mosaic())
	{
		if (auto Subsystem = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
			Subsystem->ShowNotificationTemplate(EDevNotificationTemplate::NotImplemented);
		return;
	}
}

void UStartGameUIBase::ExchangeButtonStyle()
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

void UStartGameUIBase::RefreshSampleMaterial()
{
	if (auto* Settings = UOperatorUISettings::Get())
	{
		if (auto Material = Settings->GetMaterial(GameMode))
		{
			SampleImage->SetBrushFromMaterial(Material);
		}
	}
}

void UStartGameUIBase::RefreshSampleClarity()
{
	if (!SampleImage || !LevelSlider || !LevelValueText) return;

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
