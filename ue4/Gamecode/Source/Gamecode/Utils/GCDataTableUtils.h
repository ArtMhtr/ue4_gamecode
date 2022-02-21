// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"

namespace GCDataTableUtils
{
	FItemTableRowBase* FindInventoryItemInfoByPath(FName ItemID, const TCHAR* DataTablePath);

	FWeaponTableRowBase* FindWeaponItemDataInfo(FName WeaponId);
	FAmmoTableRowBase* FindAmmoItemDataInfo(FName AmmoId);
	FItemTableRowBase* FindConsumableItemDataInfo(FName ConsumableId);

	FItemTableRowBase* FindInventoryItemDataInfoByType(FName ItemId, EInventoryItemType InventoryItemType);
	TTuple<FName, FAmmoTableRowBase*> FindAmmoItemDataInfoByAmmunitionType(EAmmunitionType AmmunitionType);
};
