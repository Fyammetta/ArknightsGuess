// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "OperatorNameMatchingDecorator.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UOperatorNameMatchingDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditDefaultsOnly)
	FSlateColor ColorAndOpacity;
	
	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};
