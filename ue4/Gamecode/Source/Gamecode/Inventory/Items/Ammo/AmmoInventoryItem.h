// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "GameCodeTypes.h"
#include "AmmoInventoryItem.generated.h"

UCLASS()
class GAMECODE_API UAmmoInventoryItem : public UInventoryItem
{
	GENERATED_BODY()

public:

	EAmmunitionType GetAmmunitionType() const;
	void SetAmmunitionType(EAmmunitionType AmmunitionType_In);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition type")
	EAmmunitionType AmmunitionType;
};
