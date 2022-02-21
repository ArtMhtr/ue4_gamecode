// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameCodeTypes.h"
#include <Engine/DataTable.h>
#include "InventoryItem.generated.h"

class AEquipableItem;
class APickableItem;
class UInventoryItem;
class AGCBaseCharacter;

USTRUCT(BlueprintType)
struct FInventoryItemDescription
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item description")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item description")
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FItemTableRowBase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	int32 MaxItemsInSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	int32 AmountToDrop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	EInventoryItemType InventoryItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	FInventoryItemDescription ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	TSubclassOf<APickableItem> PickableActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	TSubclassOf<UInventoryItem> IntentoryItemClass;
};

USTRUCT(BlueprintType)
struct FWeaponTableRowBase : public FItemTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	TSubclassOf<AEquipableItem> EquipableActor;
};

USTRUCT(BlueprintType)
struct FAmmoTableRowBase : public FItemTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo view")
	EAmmunitionType AmmunitionType;
};

UCLASS(Blueprintable)
class GAMECODE_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:

	void Initialize(FName ItemId_In, const FInventoryItemDescription& InventoryItemDescription_In, int32 MaxSlotCapacity_In, EInventoryItemType InventoryItemType, bool bIsConsumable, bool bIsEquipable);

	virtual bool IsEquipable() const;
	virtual bool IsConsumable() const;
	virtual bool Consume(AGCBaseCharacter* ConsumeTarget);

	const FName& GetItemId() const;
	EInventoryItemType GetItemType() const;

	int32 GetMaxItemsInSlot() const;
	const FInventoryItemDescription& GetDescription() const;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	EInventoryItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	FInventoryItemDescription Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	bool bIsEquipable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	bool bIsConsumable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	int32 MaxSlotCapacity = 1;

private:
	bool bIsInitialized = false;
};
