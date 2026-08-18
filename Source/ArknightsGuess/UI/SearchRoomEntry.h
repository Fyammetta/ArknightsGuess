// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "SearchRoomEntry.generated.h"

class UImage;
class UTextBlock;
class UButton;

/**
 * A single row in the room-search result list.
 */
UCLASS()
class ARKNIGHTSGUESS_API USearchRoomEntry : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ---- Widgets ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_Ping;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerNum;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoomName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Select;

	// The session this entry stands for.
	// Plain member on purpose: FOnlineSessionSearchResult is not a reflectable type.
	FOnlineSessionSearchResult Target;

public:
	/** Init the display from a search result. Called by UGameMainUI::OnLocalServerSearchComplete. */
	void InitEntry(const FOnlineSessionSearchResult& InResult);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button_Select click: resolve the session address and travel to it. */
	UFUNCTION()
	void OnSelectClicked();
};
