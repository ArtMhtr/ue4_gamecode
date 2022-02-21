// Fill out your copyright notice in the Description page of Project Settings.


#include "GCDataTableUtils.h"
#include "GameCodeTypes.h"
#include "Inventory/Items/InventoryItem.h"

const TCHAR* DataTableWeapons = TEXT("/Game/Gamecode/Core/Data/DataTables/DT_WeaponList.DT_WeaponList");
const TCHAR* DataTableItems = TEXT("/Game/Gamecode/Core/Data/DataTables/DT_InventoryItemList.DT_InventoryItemList");
const TCHAR* DataTablePickableAmmo = TEXT("/Game/Gamecode/Core/Data/DataTables/DT_PickableAmmoList.DT_PickableAmmoList");

FItemTableRowBase* GCDataTableUtils::FindInventoryItemInfoByPath(FName ItemID, const TCHAR* DataTablePath)
{
	static const FString ContextString(TEXT("Find inventory items data"));
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, DataTablePath);
	if (DataTable == nullptr)
	{
		return nullptr;
	}

	return DataTable->FindRow<FItemTableRowBase>(ItemID, ContextString);
}

FWeaponTableRowBase* GCDataTableUtils::FindWeaponItemDataInfo(FName WeaponId)
{
	return StaticCast<FWeaponTableRowBase*>(FindInventoryItemInfoByPath(WeaponId, DataTableWeapons));
}

FAmmoTableRowBase* GCDataTableUtils::FindAmmoItemDataInfo(FName AmmoId)
{
	return StaticCast<FAmmoTableRowBase*>(FindInventoryItemInfoByPath(AmmoId, DataTablePickableAmmo));
}

FItemTableRowBase* GCDataTableUtils::FindConsumableItemDataInfo(FName ConsumableId)
{
	return FindInventoryItemInfoByPath(ConsumableId, DataTableItems);
}

FItemTableRowBase* GCDataTableUtils::FindInventoryItemDataInfoByType(FName ItemId, EInventoryItemType InventoryItemType)
{
	switch (InventoryItemType)
	{
	case EInventoryItemType::Consumables:
		return FindConsumableItemDataInfo(ItemId);
	case EInventoryItemType::Ammo:
		return FindAmmoItemDataInfo(ItemId);
	case EInventoryItemType::Weapon:
		return FindWeaponItemDataInfo(ItemId);
	default:
		check(false);
		return nullptr;
	}
}

TTuple<FName, FAmmoTableRowBase*> GCDataTableUtils::FindAmmoItemDataInfoByAmmunitionType(EAmmunitionType AmmunitionType)
{
	TTuple<FName, FAmmoTableRowBase*> Result {};
	static const FString ContextString(TEXT("Find pickable ammo data"));
	UDataTable* AmmoDataTable = LoadObject<UDataTable>(nullptr, DataTablePickableAmmo);
	if (AmmoDataTable == nullptr)
	{
		return Result;
	}
	TArray<FName> RowIDs = AmmoDataTable->GetRowNames();
	TArray<FAmmoTableRowBase*> Rows;
	AmmoDataTable->GetAllRows(ContextString, Rows);

	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		FAmmoTableRowBase* CurrentRow = Rows[i];
		if (CurrentRow->AmmunitionType == AmmunitionType)
		{
			Result.Key = RowIDs[i];
			Result.Value = CurrentRow;
		}
	}
	return Result;
}