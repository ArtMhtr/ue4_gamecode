// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInventoryComponent.h"
#include "UI/Widget/Inventory/InventoryViewWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/InventorySlot.h"
#include "Inventory/Items/Ammo/AmmoInventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "Inventory/InventoryItemFactory.h"
#include <Algo/Transform.h>
#include "Characters/GCBaseCharacter.h"
#include "Actors/Interactive/Pickables/PickableItem.h"

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (!InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->AddToViewport();
	}
}

void UCharacterInventoryComponent::CloseViewInventory()
{
	if (InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->RemoveFromParent();
	}
}

bool UCharacterInventoryComponent::IsViewVisible() const
{
	bool bResult = false;
	if (IsValid(InventoryViewWidget))
	{
		bResult = InventoryViewWidget->IsVisible();
	}
	return bResult;
}

int32 UCharacterInventoryComponent::GetCapacity() const
{
	return Capacity;
}

bool UCharacterInventoryComponent::HasFreeSlot() const
{
	return ItemsInInventory < Capacity;
}

uint32 UCharacterInventoryComponent::AddItems(FName ItemId, EInventoryItemType InventoryItemType, uint32 Count)
{
	// trying to refill existing slots from left to right
	for (UInventorySlot* CurrentSlot : InventorySlots)
	{
		if (CurrentSlot->HasCompatibleItem(ItemId, InventoryItemType))
		{
			uint32 FreeSpace = GetFreeSpaceInSlot(CurrentSlot);
			if (FreeSpace)
			{
				uint32 ItemsToAddInSlot = FMath::Min(Count, FreeSpace);
				CurrentSlot->Count += ItemsToAddInSlot;
				Count -= ItemsToAddInSlot;
				CurrentSlot->UpdateSlotState();
			}
		}
	}

	// if some items didn't fill into existing slots, we use free ones
	while (HasFreeSlot() && Count > 0)
	{
		FItemTableRowBase* ItemInfo = GCDataTableUtils::FindInventoryItemDataInfoByType(ItemId, InventoryItemType);
		UInventorySlot* FreeSlot = FindFreeSlot();

		if (IsValid(FreeSlot))
		{
			FreeSlot->Item = UInventoryItemFactory::CreateInventoryItem(GetOuter(), ItemId, InventoryItemType);
			if (!IsValid(FreeSlot->Item))
			{
				return Count;
			}

			uint32 ItemsToAddInSlot = FMath::Min(Count, (uint32)ItemInfo->MaxItemsInSlot);
			FreeSlot->Count = ItemsToAddInSlot;
			Count -= ItemsToAddInSlot;

			ItemsInInventory++;
			FreeSlot->UpdateSlotState();
		}
	}

	return Count;
}

void UCharacterInventoryComponent::RemoveItems(FName ItemId, EInventoryItemType InventoryItemType, int32 Count)
{
	// trying to delete items from right to left
	for (int32 i = InventorySlots.Num() - 1; i >= 0; --i)
	{
		UInventorySlot* CurrentSlot = InventorySlots[i];
		if (CurrentSlot->HasCompatibleItem(ItemId, InventoryItemType))
		{
			int32 ItemsToRemoveFromSlot = FMath::Min(Count, CurrentSlot->Count);
			CurrentSlot->Count -= ItemsToRemoveFromSlot;
			Count -= ItemsToRemoveFromSlot;

			if (CurrentSlot->Count == 0)
			{
				ItemsInInventory--;
				CurrentSlot->ClearSlot();
			}
			else
			{
				CurrentSlot->UpdateSlotState();
			}

			if (Count == 0)
			{
				break;
			}
		}
	}
}

void UCharacterInventoryComponent::DropItemsFromSlot(UInventorySlot* Slot)
{
	if (!IsValid(Slot))
	{
		return;
	}

	FName ItemId = Slot->Item->GetItemId();
	EInventoryItemType InventoryItemType = Slot->Item->GetItemType();

	FItemTableRowBase* ItemInfo = GCDataTableUtils::FindInventoryItemDataInfoByType(ItemId, InventoryItemType);
	if (CachedBaseCharacter.IsValid() && ItemInfo != nullptr)
	{
		const float SpawnOffset = 100.0f;
		FVector	SpawnLocation = CachedBaseCharacter->GetActorLocation() + CachedBaseCharacter->GetActorForwardVector() * SpawnOffset;

		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APickableItem* DroppedItem = GetWorld()->SpawnActor<APickableItem>(ItemInfo->PickableActor, FTransform(SpawnLocation), ActorSpawnParameters);
		if (IsValid(DroppedItem))
		{
			uint32 ItemsToDrop = FMath::Min(Slot->Count, ItemInfo->AmountToDrop);
			checkf(ItemsToDrop, TEXT("Proper amount to drop from slot should be set in data tables"));

			RemoveItems(ItemId, InventoryItemType, ItemsToDrop);
			DroppedItem->SetItemsCount(ItemsToDrop);
		}
	}
}

TArray<UInventorySlot*> UCharacterInventoryComponent::GetAllItemsCopy() const
{
	return InventorySlots;
}

TArray<FText> UCharacterInventoryComponent::GetAllItemsNames() const
{
	TArray<FText> Result;
	Algo::TransformIf
	(
		InventorySlots, Result,
		[=](const UInventorySlot* InventorySlot) { return IsValid(InventorySlot->Item); },
		[=](const UInventorySlot* InventorySlot) { return InventorySlot->Item->GetDescription().Name; }
	);
	return Result;
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	check(GetOwner()->IsA<AGCBaseCharacter>());
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());

	InventorySlots.AddDefaulted(Capacity);
	for (int32 i = 0; i < Capacity; i++)
	{
		InventorySlots[i] = NewObject<UInventorySlot>(GetOwner());
	}
}

void UCharacterInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int32 i = 0; i < Capacity; i++)
	{
		InventorySlots[i]->ConditionalBeginDestroy();
	}
}	

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (IsValid(InventoryViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !IsValid(InventoryViewWidgetClass))
	{
		return;
	}

	InventoryViewWidget = CreateWidget<UInventoryViewWidget>(PlayerController, InventoryViewWidgetClass);
	InventoryViewWidget->InitializeViewWidget(InventorySlots);
}

UInventorySlot* UCharacterInventoryComponent::FindItemSlot(EInventoryItemType InventoryItemType)
{
	return *(InventorySlots.FindByPredicate([=](const UInventorySlot* InventorySlot) { return InventorySlot->Item->GetItemType() == InventoryItemType; }));
}

UInventorySlot* UCharacterInventoryComponent::FindFreeSlot()
{
	return *(InventorySlots.FindByPredicate([=](const UInventorySlot* InventorySlot) { return !IsValid(InventorySlot->Item) && InventorySlot->Count == 0; }));
}

uint32 UCharacterInventoryComponent::GetFreeSpaceInSlot(const UInventorySlot* InventorySlot) const
{	
	return InventorySlot->Item->GetMaxItemsInSlot() - InventorySlot->Count;
}

