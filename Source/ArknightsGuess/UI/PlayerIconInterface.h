// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerIconInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType)
class UPlayerIconInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARKNIGHTSGUESS_API IPlayerIconInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual UTexture2D* GetPlayerIcon() const = 0;
	
	virtual void ChangePlayerIcon(UTexture2D* Icon) = 0;
	
	virtual void Select(bool bSelected){};
	
	virtual TMulticastDelegate<void()>* OnPlayerIconChanged(){ return nullptr; };
	
	virtual bool ShouldShowIcon() { return true; };
};
