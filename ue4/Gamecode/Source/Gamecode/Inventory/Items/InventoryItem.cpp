// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Equipables/WeaponInventoryItem.h"

void UInventoryItem::Initialize(FName ItemId_In, const FInventoryItemDescription& InventoryItemDescription_In, int32 MaxSlotCapacity_In, EInventoryItemType InventoryItemType, bool bIsConsumable_In, bool bIsEquipable_In)
{
	ItemId = ItemId_In;
	ItemType = InventoryItemType;
	
	Description.Icon = InventoryItemDescription_In.Icon;
	Description.Name = InventoryItemDescription_In.Name;

	MaxSlotCapacity = MaxSlotCapacity_In;

	bIsConsumable = bIsConsumable_In;
	bIsEquipable = bIsEquipable_In;

	bIsInitialized = true;
}

bool UInventoryItem::IsEquipable() const
{
	return bIsEquipable;
}

const FInventoryItemDescription& UInventoryItem::GetDescription() const
{
	return Description;
}

bool UInventoryItem::IsConsumable() const
{
	return bIsConsumable;
}

bool UInventoryItem::Consume(AGCBaseCharacter* ConsumeTarget)
{
	return false;
}

const FName& UInventoryItem::GetItemId() const
{
	return ItemId;
}

int32 UInventoryItem::GetMaxItemsInSlot() const
{
	return MaxSlotCapacity;
}

EInventoryItemType UInventoryItem::GetItemType() const
{
	return ItemType;
}

