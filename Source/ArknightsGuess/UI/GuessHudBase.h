// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuessHudBase.generated.h"

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
	
	
};
