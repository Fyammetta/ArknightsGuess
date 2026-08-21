// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StartGameUIBase.h"
#include "MultiplayRoomUI.generated.h"

class USlider;
class UButton;
class UImage;
class UTextBlock;
class UTileView;
class UWidgetSwitcher;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UMultiplayRoomUI : public UStartGameUIBase
{
	GENERATED_BODY()
	int32 Size = 4;
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FText> GameModeTextMapping;
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* HostCastSwitcher;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTileView* PlayerList;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PORT_DisplayText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* IP_DisplayText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* PrepareButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_GameMode;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_DefaultLevel;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_HintFrequency;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_ShuffleLimit;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_GuessCount;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* QuitGameButton;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Bind shared + Room-specific buttons (base binds Mosaic/Part/Start)
	virtual void BindButtons() override;

	// ---- Start button (overridden behaviour) ----
	virtual void OnStartGameClicked() override;

	// ---- Slider callbacks (Host) broadcast to Cast ----
	virtual void OnLevelSliderChanged(float Value) override;
	virtual void OnGuessCountSliderChanged(float Value) override;
	virtual void OnShuffleLimitSliderChanged(float Value) override;
	virtual void OnHintFreqSliderChanged(float Value) override;

	// ---- Guess mode switching (Host, mirror of UGameMainUI) ----
	virtual void OnMosaicModeClicked() override;
	virtual void OnPartModeClicked() override;
	
	UFUNCTION()
	virtual void OnQuitRoomClicked();

private:
	bool UpdateSize();
	
	float GetDesiredHeight() const;


	UFUNCTION()
	void OnPlayerJoinedOrLeft(APlayerState* Player = nullptr, bool bWasJoined = true);

	// ---- Button callbacks ----
	UFUNCTION()
	void OnPrepareClicked();

	// ---- Cast: receives settings broadcast from Host ----
	UFUNCTION()
	void OnGameSettingChanged(FGameplayTag SettingTag, const FString& NewValue);

	void BroadcastSetting(const FGameplayTag& SettingTag, int32 Value);
	
};
