// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HintTextUnit.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class ARKNIGHTSGUESS_API UHintTextUnit : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Background;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Hint;
	

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TMap<FString, FLinearColor> ColorMap;
	
	FString Type;
	
	FString Text;
public:
	void SetUpTextUnit(const FString& Info);
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
