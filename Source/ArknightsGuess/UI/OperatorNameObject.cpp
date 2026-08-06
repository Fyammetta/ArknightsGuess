// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorNameObject.h"


void UOperatorNameObject::OnUserInputName(const FText& Text)
{
	FString Input = Text.ToString();
	FString Name = OriginText.ToString();
	if (!Text.IsEmpty() && Name.Contains(Input))
	{
		FString Left{};
		FString Right{};
		Name.Split(Input,&Left,&Right);
		
		RichText = Left + TEXT("<Highlight>") + Input + TEXT("</>") + Right;
	}
	else
	{
		RichText.Empty();
	}
}

FText UOperatorNameObject::GetModifiedRichText() const
{
	FText ModifiedText = FText::FromString(RichText);
	return ModifiedText;
}

bool UOperatorNameObject::GetShouldDisplay() const
{
	return !RichText.IsEmpty();
}

FEntryConfirmDelegate& UOperatorNameObject::Init(const FName& Origin, TMulticastDelegate<void(const FText&)>& Delegate)
{
	OriginText = Origin;
	RichText.Empty();
	Delegate.AddUObject(this, &UOperatorNameObject::OnUserInputName);
	return OnEntryConfirmOperator;
}

void UOperatorNameObject::OnConfirm()
{
	OnEntryConfirmOperator.ExecuteIfBound(OriginText);
}
