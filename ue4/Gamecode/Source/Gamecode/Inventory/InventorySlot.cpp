// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlot.h"
#include "Items/InventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Actors/Interactive/Pickables/PickableItem.h"
#include "Characters/GCBaseCharacter.h"

bool UInventorySlot::HasItem() const
{
	return IsValid(Item) && Count > 0;
}

bool UInventorySlot::HasCompatibleItem(FName ItemId, EInventoryItemType InventoryItemType)
{
	return HasItem() && (InventoryItemType == Item->GetItemType()) && (ItemId ==  Item->GetItemId());
}

void UInventorySlot::BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const
{
	OnInventorySlotUpdate = Callback;
}

void UInventorySlot::UnbindOnInventorySlotUpdate()
{
	OnInventorySlotUpdate.Unbind();
}

void UInventorySlot::UpdateSlotState()
{
	if (Count == 0)
	{
		Item = nullptr;
	}
	OnInventorySlotUpdate.ExecuteIfBound();
}

void UInventorySlot::ClearSlot()
{
	Item = nullptr;
	Count = 0;
	UpdateSlotState();
}