// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBarrelComponent.h"
#include "GameCodeTypes.h"
#include "Subsystems/DebugSubsystem.h"
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include <Components/DecalComponent.h>
#include <NiagaraComponent.h>
#include <NiagaraFunctionLibrary.h>
#include "Actors/Projectiles/GCProjectile.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include <Net/UnrealNetwork.h>

bool UWeaponBarrelComponent::HitScan(FVector ShortStart, OUT FVector& ShotEnd, FVector ShotDirection)
{
	bool bHasHit = false;

	FHitResult ShotResult;
	bHasHit = GetWorld()->LineTraceSingleByChannel(ShotResult, ShortStart, ShotEnd, ECC_Bullet);

	if (bHasHit)
	{
		ProcessHit(ShortStart, ShotResult, ShotDirection);
		ShotEnd = ShotResult.ImpactPoint;
	}	

	return bHasHit;
}

void UWeaponBarrelComponent::LaunchProjectile(FVector LaunchStart, FVector LaunchDirection)
{
	AGCProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];
	if (IsValid(Projectile))
	{
		Projectile->SetActorLocation(LaunchStart);
		Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
		Projectile->SetProjectileActive(true);
		Projectile->OnProjectileHit.AddDynamic(this, &UWeaponBarrelComponent::ProcessProjectileHit);
		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
		CurrentProjectileIndex++;
		if (CurrentProjectileIndex == ProjectilePool.Num())
		{
			CurrentProjectileIndex = 0;
		}
	}
}

UWeaponBarrelComponent::UWeaponBarrelComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWeaponBarrelComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.Condition = COND_SimulatedOnly;
	RepParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarrelComponent, LastShotsInfo, RepParams);

	DOREPLIFETIME(UWeaponBarrelComponent, ProjectilePool);
	DOREPLIFETIME(UWeaponBarrelComponent, CurrentProjectileIndex);
}

void UWeaponBarrelComponent::Shot(FVector ShortStart, FVector ShotDirection, float SpreadAngle)
{
	TArray<FShotInfo> ShotsInfo;
	for (int i = 0; i < CurrentFireModeSettings.BulletsPerShot; i++) {
		ShotDirection += GetBulletSpreadOffset(FMath::RandRange(0.0f, SpreadAngle), ShotDirection.ToOrientationRotator());
		ShotDirection = ShotDirection.GetSafeNormal();

		ShotsInfo.Emplace(ShortStart, ShotDirection);
	}

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_Shot(ShotsInfo);
	}

	ShotInternal(ShotsInfo);
}

void UWeaponBarrelComponent::SetFireModeSettings(const FFireModeSettings& NewFireModeSettings)
{
	CurrentFireModeSettings = NewFireModeSettings;
	FillProjectilePool(CurrentFireModeSettings);
}

void UWeaponBarrelComponent::ShotInternal(const TArray<FShotInfo>& ShotsInfo)
{
	if (GetOwner()->HasAuthority())
	{
		LastShotsInfo = ShotsInfo;
	}

	FVector MuzzleLocation = GetComponentLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentFireModeSettings.MuzzleFlashFX, MuzzleLocation, GetComponentRotation());

	for (const FShotInfo& ShotInfo : ShotsInfo)
	{

		FVector ShotStart = ShotInfo.GetLocation();
		FVector ShotDirection = ShotInfo.GetDirection();
		FVector ShotEnd = ShotStart + CurrentFireModeSettings.FiringRange * ShotDirection;

		bool bHasHit = false;
		switch (CurrentFireModeSettings.HitRegistrationType)
		{
		case EHitRegistrationType::HitScan:
		{
			bHasHit = HitScan(ShotStart, ShotEnd, ShotDirection);
			break;
		}
		case EHitRegistrationType::Projectile:
		{
			LaunchProjectile(ShotStart, ShotDirection);
			break;
		}
		}

#if ENABLE_DRAW_DEBUG
		UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryRangeWeapon);
#else
		bool bIsDebugEnabled = false;
#endif

		if (bIsDebugEnabled && bHasHit)
		{
			DrawDebugSphere(GetWorld(), ShotEnd, 10.0f, 24, FColor::Red, false, 1.0f);
			DrawDebugLine(GetWorld(), MuzzleLocation, ShotEnd, FColor::Red, false, 1.0f, 0, 3.0f);
		}

		if (CurrentFireModeSettings.TraceFX)
		{
			UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentFireModeSettings.TraceFX, MuzzleLocation, GetComponentRotation());
			if (IsValid(TraceFXComponent))
			{
				TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);
			}
		}
	}
}

void UWeaponBarrelComponent::Server_Shot_Implementation(const TArray<FShotInfo>& ShotsInfo)
{
	ShotInternal(ShotsInfo);
}

void UWeaponBarrelComponent::OnRep_LastShotsInfo()
{
	ShotInternal(LastShotsInfo);
}

FVector UWeaponBarrelComponent::GetBulletSpreadOffset(float Angle, FRotator ShotRotation)
{
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.0f, 2 * PI);

	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	FVector Result = ShotRotation.RotateVector(FVector::UpVector) * SpreadZ + ShotRotation.RotateVector(FVector::RightVector) * SpreadY;

	return Result * SpreadSize;
}

void UWeaponBarrelComponent::ProcessProjectileHit(AGCProjectile* Projectile, const FVector& ShotStartLocation, const FHitResult& Hit, const FVector& Direction)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);
	ProcessHit(ShotStartLocation, Hit, Direction);
}

void UWeaponBarrelComponent::ProcessHit(const FVector& ShortStartLocation, const FHitResult& HitResult, const FVector& Direction)
{
	AActor* HitActor = HitResult.GetActor();
	if (GetOwner()->HasAuthority() && IsValid(HitActor))
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = HitResult;
		DamageEvent.ShotDirection = Direction;
		DamageEvent.DamageTypeClass = CurrentFireModeSettings.DamageTypeClass;

		float CurrentDamageAmount = CurrentFireModeSettings.DamageAmount;
		if (IsValid(CurrentFireModeSettings.DamageFalloffDiagram))
		{
			// fall damage diagram is in meters
			float DistanceToTargetInMeters = (HitResult.ImpactPoint - ShortStartLocation).Size() / 100.0f;
			float DamageCoef = CurrentFireModeSettings.DamageFalloffDiagram->GetFloatValue(DistanceToTargetInMeters);
			CurrentDamageAmount *= DamageCoef;
		}

		HitActor->TakeDamage(CurrentDamageAmount, DamageEvent, GetController(), GetOwner());
	}

	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), CurrentFireModeSettings.DefaultDecalInfo.DecalMaterial,
		CurrentFireModeSettings.DefaultDecalInfo.DecalSize,
		HitResult.ImpactPoint,
		HitResult.ImpactNormal.ToOrientationRotator());

	if (IsValid(DecalComponent))
	{
		DecalComponent->SetFadeScreenSize(0.0001f);
		DecalComponent->SetFadeOut(CurrentFireModeSettings.DefaultDecalInfo.DecalLifeTime, CurrentFireModeSettings.DefaultDecalInfo.DecalFadeOutTime);
	}
}

void UWeaponBarrelComponent::FillProjectilePool(const FFireModeSettings& FireModeSettings)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	ProjectilePool.Empty();
	if (!IsValid(FireModeSettings.ProjectileClass))
	{
		return;
	}

	ProjectilePool.Reserve(CurrentFireModeSettings.ProjectilePoolSize);

	for (int32 i = 0; i < CurrentFireModeSettings.ProjectilePoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(CurrentFireModeSettings.ProjectileClass, ProjectilePoolLocation, FRotator::ZeroRotator);

		if (IsValid(Projectile))
		{
			Projectile->SetOwner(GetOwningPawn());
			Projectile->SetProjectileActive(false);
			ProjectilePool.Add(Projectile);
		}
	}
}

APawn* UWeaponBarrelComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}

	return PawnOwner;
}

AController* UWeaponBarrelComponent::GetController() const
{
	APawn* PawnOwner = GetOwningPawn();
	if (IsValid(PawnOwner))
	{
		return PawnOwner->GetController();
	}
	else
	{
		return nullptr;
	}
}

