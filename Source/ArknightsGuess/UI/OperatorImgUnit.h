// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Operators/OperatorTypes.h"
#include "OperatorImgUnit.generated.h"

struct FOperatorImage;
class UWrapBox;
class UImage;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UOperatorImgUnit : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Image;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWrapBox* HintBox;
	
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void SetupUnit(const FOperatorImage& Img, const TArray<FString>& Hints);
	
	UFUNCTION()
	void OnCallClarify(int32 Round, int32 Level);
	
	UFUNCTION()
	void OnCheckAnswer(EGuessRoundState RoundState);

	UFUNCTION()
	void OnShowNextHint();

	int32 CurrentHintIndex = 0;
};
