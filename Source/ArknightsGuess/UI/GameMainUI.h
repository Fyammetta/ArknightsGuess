// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMainUI.generated.h"

class UButton;
class UWidgetSwitcher;
class USlider;
class UTextBlock;
class UImage;
class UBorder;
class UWidgetAnimation;

/**
 * Main menu / settings UI for the operator guessing game.
 */


UCLASS()
class ARKNIGHTSGUESS_API UGameMainUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ---- Play mode buttons ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SoloPlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MultiPlayButton;

	// ---- Sub-widget switcher ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SubWidgetSwitcher;

	// ---- Settings sliders (match UOperatorUISettings int32 values) ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> LevelSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> GuessCountSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> ShuffleLimitSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> HintFreqSlider;

	// ---- Slider value labels ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> GuessCountValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ShuffleLimitValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HintFreqValueText;

	// ---- Guess mode buttons ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MosaicModeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> PartModeButton;

	// ---- Start button ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	// ---- Sample image ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> SampleImage;

	// ---- Settings container ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> SettingsBorder;

	// ---- Animation ----
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ShowSettingsWidget;

	// ---- State ----
	bool bExpandedSettings = false;
	
	FName GameMode = TEXT("Mosaic");

public:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// ---- Slider callbacks ----
	UFUNCTION()
	void OnLevelSliderChanged(float Value);

	UFUNCTION()
	void OnGuessCountSliderChanged(float Value);

	UFUNCTION()
	void OnShuffleLimitSliderChanged(float Value);

	UFUNCTION()
	void OnHintFreqSliderChanged(float Value);

	// ---- Button callbacks ----
	UFUNCTION()
	void OnSoloPlayClicked();

	UFUNCTION()
	void OnMultiPlayClicked();

	UFUNCTION()
	void OnMosaicModeClicked();

	UFUNCTION()
	void OnPartModeClicked();

	UFUNCTION()
	void OnStartGameClicked();

private:
	void BindSliders();
	void BindButtons();
};
