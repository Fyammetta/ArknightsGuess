// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
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
class ARKNIGHTSGUESS_API UMultiplayRoomUI : public UUserWidget
{
	GENERATED_BODY()
	FTimerHandle TimerHandle;
	int32 Size = 4;
	bool bShowGameplayLevel = true;
	FGameplayTag GameMode;
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FText> GameModeTextMapping;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* HostCastSwitcher;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTileView* PlayerList;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PORT_DisplayText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* IP_DisplayText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* SampleImage;
	
	/// ======================
	///	======== Host ========
	/// ======================
	/// 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> LevelSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> GuessCountSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> ShuffleLimitSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> HintFreqSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> GuessCountValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ShuffleLimitValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HintFreqValueText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* StartGameButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* MosaicModeButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* PartModeButton;
	
	/// ======================
	///	======== Cast ========
	/// ======================
	/// 
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
	UButton* PrepareButton;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
private:
	bool UpdateSize();
	
	float GetDesiredHeight() const;


	UFUNCTION()
	void OnPlayerJoinedOrLeft(APlayerState* Player = nullptr, bool bWasJoined = true);

	// ---- Slider callbacks (Host) ----
	UFUNCTION()
	void OnLevelSliderChanged(float Value);

	UFUNCTION()
	void OnGuessCountSliderChanged(float Value);

	UFUNCTION()
	void OnShuffleLimitSliderChanged(float Value);

	UFUNCTION()
	void OnHintFreqSliderChanged(float Value);

	// ---- Button callbacks (bind only, body left empty) ----
	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnPrepareClicked();

	// ---- Cast: receives settings broadcast from Host ----
	UFUNCTION()
	void OnGameSettingChanged(FGameplayTag SettingTag, const FString& NewValue);

	void BindSliders();
	void BindButtons();
	void RefreshSampleMaterial();
	void RefreshSampleClarity();
	void BroadcastSetting(const FGameplayTag& SettingTag, int32 Value);
	
};
