// Fill out your copyright notice in the Description page of Project Settings.

#include "HintTextUnit.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHintTextUnit::SetUpTextUnit(const FString& Info)
{
	FString HintType{};
	FString HintText{};
	
	Info.Split("/",&HintType, &HintText);
	
	Hint->SetText(FText::FromString(HintText));
	if (ColorMap.Contains(HintType))
	{
		Hint->SetColorAndOpacity(ColorMap[HintType].Text);
		Background->SetColorAndOpacity(ColorMap[HintType].Background);
	}
}
