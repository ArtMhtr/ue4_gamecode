// Fill out your copyright notice in the Description page of Project Settings.


#include "BowAnimInstance.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"

void UBowAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	check(GetOwningActor()->IsA<ARangeWeaponItem>());
	CachedBow = StaticCast<ARangeWeaponItem*>(GetOwningActor());
}

void UBowAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (!CachedBow.IsValid())
	{
		return;
	}

	bIsAiming = CachedBow->IsAiming();
}
