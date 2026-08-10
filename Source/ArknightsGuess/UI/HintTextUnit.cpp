// Fill out your copyright notice in the Description page of Project Settings.

#include "HintTextUnit.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHintTextUnit::SetUpTextUnit(const FString& Info)
{
	Info.Split("/",&Type, &Text);
	if (Type.Contains(TEXT("tag")))
		Type = TEXT("标签");
	
	Hint->SetText(FText::FromString(Text));
	if (ColorMap.Contains(Type))
	{
		Background->SetColorAndOpacity(ColorMap[Type]);
	}
}

void UHintTextUnit::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	Hint->SetText(FText::FromString(Type));

}

void UHintTextUnit::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	Hint->SetText(FText::FromString(Text));

}
