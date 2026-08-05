// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OperatorImgUnit.generated.h"

class UWrapBox;
struct FOperatorData;
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
	
public:
	void SetupUnit(const FOperatorData& Data);
};
