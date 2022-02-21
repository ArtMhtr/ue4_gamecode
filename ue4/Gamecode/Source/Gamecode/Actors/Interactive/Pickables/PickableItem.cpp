// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Characters/GCBaseCharacter.h"

APickableItem::APickableItem()
{
	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionShape"));
	SetRootComponent(SphereCollisionComponent);

	PickableItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	PickableItemMesh->SetupAttachment(SphereCollisionComponent);

	SphereCollisionComponent->SetCollisionProfileName(CollisionProfilePickableItemVolume);
}

void APickableItem::Interact(AGCBaseCharacter* Character)
{
	bool bPickedUp = Character->AddItems(DataTableID, InventoryItemType, ItemsCount);
	if (bPickedUp)
	{
		Destroy();
	}
}

FName APickableItem::GetActionEventName() const
{
	return ActionInteract;
}

void APickableItem::SetItemsCount(int32 ItemsCount_In)
{
	ItemsCount = ItemsCount_In;
}
