// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableItem.h"
#include "Characters/GCBaseCharacter.h"
#include "Actors/Projectiles/GCProjectile.h"
#include <GameFramework/PlayerController.h>
#include <Math/UnrealMathUtility.h>

void AThrowableItem::Throw(AGCProjectile* Projectile, FVector LaunchStartLocation, FVector LaunchDirection)
{
	if (IsValid(Projectile))
	{
		Projectile->SetActorLocation(LaunchStartLocation);
		Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());

		Projectile->SetProjectileActive(true);

		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
	}
}

EAmmunitionType AThrowableItem::GetAmmoType() const
{
	return AmmoType;
}

TSubclassOf<class AGCProjectile> AThrowableItem::GetProjectileClass() const
{
	return ProjectileClass;
}

float AThrowableItem::GetThrowAngle() const
{
	return ThrowAngle;
}
