// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerIconEntry.h"

#include "PlayerIconInterface.h"
#include "Components/Image.h"


void UPlayerIconEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	if (ListItemObject->Implements<UPlayerIconInterface>())
	{	
		/*if (Object.IsValid())
			Object->SetObjectValid(false);*/
		

		Object = Cast<IPlayerIconInterface>(ListItemObject);
		Icon->SetBrushFromTexture(Object->GetPlayerIcon());
		//Object->SetObjectValid(true);
	}
}

void UPlayerIconEntry::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);
	
	if (Object.IsValid() /*&& Object->IsObjectValid()*/)
	{
		Object->Select(bIsSelected);
	}
}
