// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorNameMatchingDecorator.h"

#include "Components/RichTextBlock.h"

class FMatchNameDecorator : public FRichTextDecorator
{
public:
	FMatchNameDecorator(URichTextBlock* InOwner, UOperatorNameMatchingDecorator* InDecorator) : FRichTextDecorator(InOwner)
	{
		Decorator = InDecorator;
	}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		return Owner && Decorator && RunParseResult.Name == TEXT("Highlight");
	}

protected:
	virtual void CreateDecoratorText(const FTextRunInfo& RunInfo, FTextBlockStyle& InOutTextStyle, FString& InOutString) const override;

	UOperatorNameMatchingDecorator* Decorator;
};

void FMatchNameDecorator::CreateDecoratorText(const FTextRunInfo& RunInfo, FTextBlockStyle& InOutTextStyle, FString& InOutString) const
{
	/*
	InOutTextStyle = Owner->GetDefaultTextStyle();
	*/
	InOutTextStyle.ColorAndOpacity = Decorator->ColorAndOpacity;
	InOutString.Append(RunInfo.Content.ToString());
}

TSharedPtr<ITextDecorator> UOperatorNameMatchingDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShared<FMatchNameDecorator>(InOwner, this);
}
