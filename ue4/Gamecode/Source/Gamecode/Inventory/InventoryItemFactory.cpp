// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemFactory.h"
#include "Items/InventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Items/Equipables/WeaponInventoryItem.h"

UInventoryItem* UInventoryItemFactory::CreateInventoryItem(UObject* Outer, FName ItemId, EInventoryItemType InventoryItemType)
{
	UInventoryItem* CurrentItem = nullptr;
	FItemTableRowBase* ItemInfo = GCDataTableUtils::FindInventoryItemDataInfoByType(ItemId, InventoryItemType);
	if (ItemInfo != nullptr)
	{
		CurrentItem = NewObject<UInventoryItem>(Outer, ItemInfo->IntentoryItemClass);
		if (!IsValid(CurrentItem))
		{
			return nullptr;
		}

		bool bIsConsumableItem = (ItemInfo->InventoryItemType == EInventoryItemType::Consumables);
		bool bIsEquipableItem = (ItemInfo->InventoryItemType == EInventoryItemType::Weapon);

		CurrentItem->Initialize(ItemId, ItemInfo->ItemDescription, ItemInfo->MaxItemsInSlot, ItemInfo->InventoryItemType, bIsConsumableItem, bIsEquipableItem);
		if (CurrentItem->GetItemType() == EInventoryItemType::Weapon)
		{
			UWeaponInventoryItem* WeaponInventoryItem = StaticCast<UWeaponInventoryItem*>(CurrentItem);
			FWeaponTableRowBase* WeaponInfo = StaticCast<FWeaponTableRowBase*>(ItemInfo);

			WeaponInventoryItem->SetEquipWeaponClass(WeaponInfo->EquipableActor);
		}
	}

	return CurrentItem;
}
