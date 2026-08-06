// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OperatorNameObject.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_OneParam(FEntryConfirmDelegate,const FName&);

UCLASS(Blueprintable,BlueprintType)
class ARKNIGHTSGUESS_API UOperatorNameObject : public UObject
{
	GENERATED_BODY()
	
	FName OriginText;
	
	FString RichText;
	
	FEntryConfirmDelegate OnEntryConfirmOperator;
public:	
	
	void OnUserInputName(const FText& Text);

	UFUNCTION(BlueprintCallable)
	FText GetModifiedRichText() const;
	
	UFUNCTION(BlueprintCallable)
	bool GetShouldDisplay() const;
	
	FEntryConfirmDelegate& Init(const FName& Origin, TMulticastDelegate<void(const FText&)>& Delegate);
	
	UFUNCTION(BlueprintCallable)
	void OnConfirm();
};
