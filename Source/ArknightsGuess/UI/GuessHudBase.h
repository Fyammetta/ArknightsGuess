// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GuessHudBase.generated.h"

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

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOperatorImgUnit* Unit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableText* AnswerBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UListView* OperatorList;

	virtual void NativeConstruct() override;

	FInputAnswerDelegate OnPlayerInputAnswer;

	TArray<TObjectPtr<UOperatorNameObject>> AllEntries;

private:
	UFUNCTION()
	void OnAnswerConfirmed();

	UFUNCTION()
	void TryRetrieveAnswer(const FText& Text);

	UFUNCTION()
	void OnGuessStateChanged(EGuessRoundState State);
	
	void ConfirmFromList(const FName& Operator);
};
