// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorNameObject.h"
#include "Operators/OperatorTypes.h"


void UOperatorNameObject::OnUserInputName(const FText& Text)
{
	FString Input = Text.ToString();
	Input = Input.Replace(TEXT("("),TEXT(" "));
	Input = Input.Replace(TEXT(")"),TEXT(" "));
	
	
	
	FString Origin = OriginText.ToString();
	if (!Text.IsEmpty() && SearchText.Contains(Input) )
	{

		FString Prefix{};
		FString Suffix{};
		
		if (Origin.Split(Input,&Prefix,&Suffix))
			RichText = Prefix + TEXT("<Highlight>") + Input + TEXT("</>") + Suffix;
		else
			RichText = Origin;
	}
	else
	{
		RichText.Empty();
	}
}

FText UOperatorNameObject::GetModifiedRichText() const
{
	return FText::FromString(RichText);
}

bool UOperatorNameObject::GetShouldDisplay() const
{
	return !RichText.IsEmpty();
}

FEntryConfirmDelegate& UOperatorNameObject::Init(const FOperatorNamePair& Name, TMulticastDelegate<void(const FText&)>& Delegate)
{
	OriginText = Name.RealName;
	SearchText = Name.SearchName;
	RichText.Empty();
	Delegate.AddUObject(this, &UOperatorNameObject::OnUserInputName);
	return OnEntryConfirmOperator;
}

void UOperatorNameObject::OnConfirm()
{
	OnEntryConfirmOperator.ExecuteIfBound(OriginText);
}
