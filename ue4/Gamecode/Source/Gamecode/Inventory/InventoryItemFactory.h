// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameCodeTypes.h"
#include "InventoryItemFactory.generated.h"

class UInventoryItem;

UCLASS()
class GAMECODE_API UInventoryItemFactory : public UObject
{
	GENERATED_BODY()
	
public:
	static UInventoryItem* CreateInventoryItem(UObject* Outer, FName ItemId, EInventoryItemType InventoryItemType);
};
