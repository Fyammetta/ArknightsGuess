// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerIconObject.h"

UTexture2D* UPlayerIconObject::GetPlayerIcon() const
{
	return PlayerIcon;
}

void UPlayerIconObject::ChangePlayerIcon(UTexture2D* Icon)
{
	if (Icon)
		PlayerIcon = Icon;
}

void UPlayerIconObject::Select(bool bSelected)
{
	if (bSelected)
		OnPlayerIconSelected.Broadcast(PlayerIcon);
}