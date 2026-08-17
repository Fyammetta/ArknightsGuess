// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "MainSettingsUI.generated.h"

class UScrollBox;
class UTextBlock;
class USlider;
class UEditableText;
class UButton;
class UImage;
class UTileView;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UMainSettingsUI : public UUserWidget
{
	GENERATED_BODY()
	virtual void NativeConstruct() override;
	
	UPROPERTY()
	UTexture2D* PlayerIcon;
	
	FString PlayerName;
	
	TMap<FGameplayTag, float> SoundVolumeMapping;
	
	TWeakObjectPtr<UButton> CurrentSettingsButton;
	
	TWeakObjectPtr<UButton> CurrentScreenButton;
	
	int32 ResolutionIndex;
	
	TArray<FIntPoint> Resolutions;
	
	FTimerHandle ResolutionChangeTimer;
	
	EWindowMode::Type ScreenMode;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor CurrentSettingsColor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor NormalSettingsColor;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTileView* IconsTile;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* MainImage;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmIconButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* SaveButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableText* PlayerNameInput;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* MainVolumeSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* UIVolumeSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* VoiceVolumeSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* MusicVolumeSlider;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* UIVolumeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* VoiceVolumeText;	
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* MusicVolumeText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* MainVolumeText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ResolutionsBox;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FSlateFontInfo ResolutionFont;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float TextHeight;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ResolutionUpButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ResolutionDownButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* FullScreenButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* WindowedFullscreenButton;
		
		UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* WindowedButton;
	
	UFUNCTION(BlueprintCallable)
	void ClickSettingButton(UButton* Target);
	
	UFUNCTION(BlueprintCallable)
	void ClickScreenButton(UButton* Target);
private:
	UFUNCTION()
	void OnSelectedIcon(UTexture2D* Icon);

	UFUNCTION()
	void OnConfirmIconClicked();

	UFUNCTION()
	void OnSaveClicked();
	
	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnMainVolumeChanged(float Value);

	UFUNCTION()
	void OnUIVolumeChanged(float Value);

	UFUNCTION()
	void OnVoiceVolumeChanged(float Value);

	UFUNCTION()
	void OnMusicVolumeChanged(float Value);

	void ApplyVolume(const FGameplayTag& Tag, float Value);

	void InitVolumeSlider(USlider* Slider, const FGameplayTag& Tag);
	
	void GetAllAvailableResolutions();
	
	UFUNCTION()
	void OnSetResolutionUp();
	
	UFUNCTION()
	void OnSetResolutionDown();
	
	void InterpResolutionOffset();
	
	UFUNCTION()
	void OnSetFullScreenMode();
	
	UFUNCTION()
	void OnSetWindowedMode();
	
	UFUNCTION()
	void OnSetWindowedFullScreenMode();

};
