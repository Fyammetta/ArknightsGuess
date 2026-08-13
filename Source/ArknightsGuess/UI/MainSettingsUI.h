// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "MainSettingsUI.generated.h"

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
	
	UFUNCTION(BlueprintCallable)
	void ClickSettingButton(UButton* Target);
private:
	UFUNCTION()
	void OnSelectedIcon(UTexture2D* Icon);

	UFUNCTION()
	void OnConfirmIconClicked();

	UFUNCTION()
	void OnSaveClicked();

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

};
