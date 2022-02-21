// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoInventoryItem.h"
#include "GameCodeTypes.h"

EAmmunitionType UAmmoInventoryItem::GetAmmunitionType() const
{
	return AmmunitionType;
}

void UAmmoInventoryItem::SetAmmunitionType(EAmmunitionType AmmunitionType_In)
{
	AmmunitionType = AmmunitionType_In;
}
