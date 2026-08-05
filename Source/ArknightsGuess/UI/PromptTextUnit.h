// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PromptTextUnit.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPromptColorPair
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FLinearColor Text;
	
	UPROPERTY(EditDefaultsOnly)
	FLinearColor Background;
};
UCLASS()
class ARKNIGHTSGUESS_API UPromptTextUnit : public UUserWidget
{
	GENERATED_BODY()
	protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Background;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Prompt;
	
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly)
	TMap<FName, FPromptColorPair> ColorMap;
	
};
