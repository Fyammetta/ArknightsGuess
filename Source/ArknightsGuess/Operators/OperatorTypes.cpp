// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorTypes.h"

FOperatorNamePair::FOperatorNamePair(const FName& Name, const TArray<FName>& AvailableNames)
{
	RealName = Name;
	SearchName = Name.ToString() + TEXT("(*)");
	for (const FName& N : AvailableNames)
	{
		SearchName.Append(FString::Printf(TEXT("(%s)"), *N.ToString()));
	}
}

TArray<FOperatorData> FOperatorData::MakeFromDataRow(const FName& RowName, const FOperatorDataRow& Row)
{
	TArray<FOperatorData> Result;
	FOperatorData Data;

	
	Data.Name = FOperatorNamePair(RowName, Row.AvailableNames);
	Data.Info = Row.Info;
	for (const auto& Tex : Row.SkinTextures)
	{
		Data.Image = Tex;
		Result.Add(Data);
	}
		
	return Result;
}

