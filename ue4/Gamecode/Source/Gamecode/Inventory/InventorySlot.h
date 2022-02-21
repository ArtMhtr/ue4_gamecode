 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Items/InventoryItem.h"
#include "InventorySlot.generated.h"

class UInventoryItem;

UCLASS()
class GAMECODE_API UInventorySlot : public UObject
{
	GENERATED_BODY()

public:

	DECLARE_DELEGATE(FInventorySlotUpdate)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInventoryItem* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	bool HasItem() const;
	bool HasCompatibleItem(FName ItemId, EInventoryItemType InventoryItemType);
	void BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const;
	void UnbindOnInventorySlotUpdate();
	void UpdateSlotState();
	void ClearSlot();

private:

	mutable FInventorySlotUpdate OnInventorySlotUpdate;
};
