// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorUISettings.h"

UMaterial* UOperatorUISettings::GetMaterial(const FGameplayTag& Mode)
{
	if (Materials.Contains(Mode))
		return Materials[Mode].LoadSynchronous();

	return nullptr;
}
