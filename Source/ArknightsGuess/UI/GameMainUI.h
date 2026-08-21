// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StartGameUIBase.h"
#include "GameMainUI.generated.h"

class UScrollBox;
class UEditableText;
class UButton;
class UWidgetSwitcher;
class USlider;
class UTextBlock;
class UImage;
class UBorder;
class UWidgetAnimation;
class UComboBoxString;
class USearchRoomEntry;

/**
 * Main menu / settings UI for the operator guessing game.
 */


UCLASS()
class ARKNIGHTSGUESS_API UGameMainUI : public UStartGameUIBase
{
	GENERATED_BODY()

	FDelegateHandle SearchHandle;
	
protected:
	// ---- Play mode buttons ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SoloPlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> CreateRoomButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ConfirmCreateRoomButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> CancelRoomButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> JoinRoomButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ConfirmJoinRoomButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SearchRoomButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> QuitGameButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> RoomList;
	
	// ---- Sub-widget switcher ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SubWidgetSwitcher;

	// ---- Part mode controls ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> PartSelector;

	// ---- Settings container ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> SettingsBorder;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> CreateRoomBorder;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> JoinRoomBorder;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> RoomNameInputText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> PortInputText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> ServerAddressInputText;
	
	// ---- Animation ----
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ShowSettingsWidget;

	// ---- State ----
	UPROPERTY(BlueprintReadOnly)
	bool bExpandedSettings = false;

	// ---- Part details ----
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector> PartDetails;

	// ---- Room search ----
	/** Entry widget class for the room-search result list. Assigned by blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USearchRoomEntry> SearchRoomEntryClass;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Bind shared + MainMenu-specific buttons (base binds Mosaic/Part/Start)
	virtual void BindButtons() override;

	// ---- Start button (overridden behaviour) ----
	virtual void OnStartGameClicked() override;

	// ---- Part mode button (overridden for PartSelector-based part) ----
	virtual void OnPartModeClicked() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	// ---- Button callbacks ----
	UFUNCTION()
	void OnSoloPlayClicked();

	UFUNCTION()
	void OnCreateRoomClicked();
	
	UFUNCTION()
	void OnCreateRoomCanceled();
	
	UFUNCTION()
	void OnMultiCreateClicked();
	
	UFUNCTION()
	void OnMultiSearchClicked();
	
	UFUNCTION()
	void OnMultiJoinClicked();

	UFUNCTION()
	void OnQuitGameClicked();
	
	UFUNCTION()
	void OnSettingsClicked();
	
	UFUNCTION()
	void OnJoinRoomClicked();
	
	void OnLocalServerSearchComplete(bool bWasSuccessful);

	// ---- ComboBox callbacks ----
	UFUNCTION()
	void OnPartSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
