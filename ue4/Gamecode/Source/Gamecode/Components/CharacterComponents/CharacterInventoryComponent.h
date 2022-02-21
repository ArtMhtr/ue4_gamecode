// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCodeTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "CharacterInventoryComponent.generated.h"

class UInventoryItem;
class UInventoryViewWidget;
class UInventorySlot;
class UWeaponInventoryItem;
class UAmmoInventoryItem;
class AGCBaseCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void OpenViewInventory(APlayerController* PlayerController);
	void CloseViewInventory();
	bool IsViewVisible() const;

	int32 GetCapacity() const;
	bool HasFreeSlot() const;

	uint32 AddItems(FName ItemId, EInventoryItemType InventoryItemType, uint32 Count);
	void RemoveItems(FName ItemId, EInventoryItemType InventoryItemType, int32 Count);
	void DropItemsFromSlot(UInventorySlot* Slot);

	TArray<UInventorySlot*> GetAllItemsCopy() const;
	TArray<FText> GetAllItemsNames() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void CreateViewWidget(APlayerController* PlayerController);

	UInventorySlot* FindItemSlot(EInventoryItemType InventoryItemType);
	UInventorySlot* FindFreeSlot();
	uint32 GetFreeSpaceInSlot(const UInventorySlot* InventorySlot) const;

	UPROPERTY(EditAnywhere, Category = "Items")
	TArray<UInventorySlot*> InventorySlots;

	UPROPERTY(EditAnywhere, Category = "View Settings")
	TSubclassOf<UInventoryViewWidget> InventoryViewWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Settings", meta = (UIMin = 1, ClampMin = 1))
	int32 Capacity = 16;

private:

	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;

	int32 ItemsInInventory;

	TWeakObjectPtr<AGCBaseCharacter> CachedBaseCharacter;
};
