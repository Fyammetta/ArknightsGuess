// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PlayerIconInterface.h"
#include "UObject/Object.h"
#include "PlayerIconObject.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FPlayerIconSelectDelegate, UTexture2D*);

UCLASS()
class ARKNIGHTSGUESS_API UPlayerIconObject : public UObject, public IPlayerIconInterface
{
	GENERATED_BODY()

	UPROPERTY()
	UTexture2D* PlayerIcon;
public:
	FPlayerIconSelectDelegate OnPlayerIconSelected;
	
	virtual UTexture2D* GetPlayerIcon() const override;
	
	virtual void ChangePlayerIcon(UTexture2D* Icon) override;

	virtual void Select(bool bSelected) override;
};
