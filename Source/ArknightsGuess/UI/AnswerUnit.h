// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnswerUnit.generated.h"

class UTextBlock;
class UImage;
class AGuessPlayerState;
class UButton;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UAnswerUnit : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CloseButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Icon;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_PlayerName;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Answer;
	
public:
	static UAnswerUnit* MakeNewUnit(UWorld* Outer, TSubclassOf<UAnswerUnit> Class, AGuessPlayerState* Player, const FName& Answer);
};
