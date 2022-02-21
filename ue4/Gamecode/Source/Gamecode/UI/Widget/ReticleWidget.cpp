// Fill out your copyright notice in the Description page of Project Settings.


#include "ReticleWidget.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"

void UReticleWidget::OnAimingStateChanged_Implementation(bool bIsAiming)
{
	SetupCurrentReticle();
}

void UReticleWidget::OnEquippedItemChanged_Implementation(const AEquipableItem* EquippedItem)
{
	CurrentEquippedItem = EquippedItem;

	if (CurrentEquippedItem.IsValid() && CurrentEquippedItem->IsA<ARangeWeaponItem>())
	{
		const ARangeWeaponItem* CurrentWeapon = StaticCast<const ARangeWeaponItem*>(CurrentEquippedItem.Get());
		CurrentAimChangeFOVDuration = CurrentWeapon->GetChangeAimFOVDuration();
	}
	
	SetupCurrentReticle();
}

void UReticleWidget::SetupCurrentReticle()
{
	if (CurrentEquippedItem.IsValid())
	{
		CurrentReticle = CurrentEquippedItem->GetReticleType();
	}
	else
	{
		CurrentReticle = EReticleType::None;
	}
}
