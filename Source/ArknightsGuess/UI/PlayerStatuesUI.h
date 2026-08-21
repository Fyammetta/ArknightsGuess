// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Operators/OperatorTypes.h"
#include "PlayerStatuesUI.generated.h"

class UVerticalBox;
class UAnswerUnit;
class UButton;
class UListView;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UPlayerStatuesUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnswerUnit> AnswerWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UListView* PlayerList;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* AnswerBox;
	
	
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnPlayerReady(APlayerState* Player, bool bReady);
	
	void OnOtherAnswered(APlayerState* Player, const FName& Answer);
	
	UFUNCTION()
	void OnGuessStateChanged(EGuessRoundState State);
};
