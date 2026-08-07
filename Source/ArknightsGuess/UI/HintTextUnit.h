// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HintTextUnit.generated.h"

class UImage;
class UTextBlock;

USTRUCT(BlueprintType)
struct FHintColorPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FLinearColor Text;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor Background;
};

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
	TMap<FString, FHintColorPair> ColorMap;
public:
	void SetUpTextUnit(const FString& Info);
};
