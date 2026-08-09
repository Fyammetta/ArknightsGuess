// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GuessHudBase.generated.h"

class UBorder;
class UOperatorNameObject;
class UListView;
class UEditableText;
class UButton;
class UOperatorImgUnit;

DECLARE_MULTICAST_DELEGATE_OneParam(FInputAnswerDelegate, const FText&);

UCLASS()
class ARKNIGHTSGUESS_API UGuessHudBase : public UUserWidget
{
	GENERATED_BODY()
	
	bool bConfirmedFromList;
	
	bool bTryingQuit;
	
	bool bMusicSettingsExpanded;
	
	bool bPreparedForNext = true;
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOperatorImgUnit* Unit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableText* AnswerBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UListView* OperatorList;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* MusicSettingButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmQuitButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CancelQuitButton;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* QuitUI;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	UWidgetAnimation* CallMusicSelect;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim))
	UWidgetAnimation* CallQuitUI;


	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	FInputAnswerDelegate OnPlayerInputAnswer;

	UPROPERTY()
	TArray<TObjectPtr<UOperatorNameObject>> AllEntries;

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
private:
	UFUNCTION()
	void OnAnswerConfirmed();
	
	UFUNCTION()
	void OnTryingQuit();
	
	UFUNCTION()
	void OnQuitConfirmed();
	
	UFUNCTION()
	void OnQuitCanceled();

	UFUNCTION()
	void OnMusicSettingClicked();

	UFUNCTION()
	void TryRetrieveAnswer(const FText& Text);

	UFUNCTION()
	void OnGuessStateChanged(EGuessRoundState State);
	
	void ConfirmFromList(const FName& Operator);
};
