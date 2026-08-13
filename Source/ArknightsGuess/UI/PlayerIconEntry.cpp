// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerIconEntry.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"

void UPlayerIconEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
}

void UPlayerIconEntry::SetEntrySize(int32 Size)
{
	SizeBox->SetHeightOverride(Size);
	SizeBox->SetWidthOverride(Size);
	
	auto Brush = Icon->GetBrush();
	
	/*Brush.OutlineSettings.*/
}
