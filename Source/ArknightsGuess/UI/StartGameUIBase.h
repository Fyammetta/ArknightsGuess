// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "StartGameUIBase.generated.h"

class UButton;
class UImage;
class USlider;
class UTextBlock;
class UWidgetSwitcher;

/**
 * Base UI shared by UGameMainUI and UMultiplayRoomUI.
 * Holds the common guess-mode (Mosaic/Part) switcher, the shared settings
 * sliders and the start-game flow. Subclasses override the virtuals for
 * their own start behaviour / extra widgets.
 */
UCLASS(Abstract)
class ARKNIGHTSGUESS_API UStartGameUIBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// ---- Shared state ----
	UPROPERTY(BlueprintReadOnly)
	bool bShowGameplayLevel = true;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag GameMode;

	// ---- Shared sliders ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> LevelSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> GuessCountSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> ShuffleLimitSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> HintFreqSlider;

	// ---- Shared slider value labels ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> GuessCountValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ShuffleLimitValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HintFreqValueText;

	// ---- Shared guess-mode buttons ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> MosaicModeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> PartModeButton;

	// ---- Shared gamemode switcher (Mosaic / Part panels) ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ModeSwitcher;

	// ---- Shared start button ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	// ---- Shared sample image ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> SampleImage;

	// ---- Slider callbacks ----
	UFUNCTION()
	virtual void OnLevelSliderChanged(float Value);

	UFUNCTION()
	virtual void OnGuessCountSliderChanged(float Value);

	UFUNCTION()
	virtual void OnShuffleLimitSliderChanged(float Value);

	UFUNCTION()
	virtual void OnHintFreqSliderChanged(float Value);

	// ---- Guess-mode switch callbacks ----
	UFUNCTION()
	virtual void OnMosaicModeClicked();

	UFUNCTION()
	virtual void OnPartModeClicked();

	// ---- Start button callback (overridden by subclasses) ----
	UFUNCTION()
	virtual void OnStartGameClicked();

	virtual void BindSliders();
	virtual void BindButtons();
	virtual void ExchangeButtonStyle();
	virtual void RefreshSampleMaterial();
	virtual void RefreshSampleClarity();

private:
	void InitRuleSettings();
	void InitSampleImage();
};
