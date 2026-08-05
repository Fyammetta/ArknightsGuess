// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorTypes.h"

TArray<FOperatorData> FOperatorData::MakeFromDataRow(const FName& RowName, const FOperatorDataRow& Row)
{
	TArray<FOperatorData> Result;
	FOperatorData Data;
	Data.Name = RowName;
	Data.Info = Row.Info;
	for (const auto& Tex : Row.SkinTextures)
	{
		Data.Image = Tex;
		Result.Add(Data);
	}
		
	return Result;
}

