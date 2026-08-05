// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GuessHudBase.generated.h"

class UEditableText;
class UButton;
class UOperatorImgUnit;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UGuessHudBase : public UUserWidget
{
	GENERATED_BODY()
	
	protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOperatorImgUnit* Unit;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableText* AnswerBox;
	
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnAnswerConfirmed();
	
	UFUNCTION()
	void TryRetrieveAnswer(const FText& Text);
	
	UFUNCTION()
	void OnGuessStateChanged(EGuessRoundState State);
};
