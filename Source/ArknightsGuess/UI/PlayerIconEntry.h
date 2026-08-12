// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PlayerIconEntry.generated.h"

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
	
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void SetEntrySize(int32 Size);
	
};
