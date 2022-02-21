// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "WeaponInventoryItem.generated.h"

UCLASS()
class GAMECODE_API UWeaponInventoryItem : public UInventoryItem
{
	GENERATED_BODY()

public:
	UWeaponInventoryItem();

	TSubclassOf<AEquipableItem> GetEquipWeaponClass() const;
	void SetEquipWeaponClass(TSubclassOf<AEquipableItem>& WeaponClass);

protected:
	TSubclassOf<AEquipableItem> EquipWeaponClass;
	
};
