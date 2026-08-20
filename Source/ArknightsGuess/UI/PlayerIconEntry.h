// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PlayerIconEntry.generated.h"

class IPlayerIconInterface;
class UImage;
class USizeBox;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UPlayerIconEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USizeBox* SizeBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Icon;
	
	bool bIsValid;
	
	TWeakInterfacePtr<IPlayerIconInterface> Object;
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

private:
	void OnIconChanged();
};
