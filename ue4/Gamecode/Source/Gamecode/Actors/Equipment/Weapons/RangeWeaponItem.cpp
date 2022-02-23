// Fill out your copyright notice in the Description page of Project Settings.


#include "RangeWeaponItem.h"
#include "Components/Weapon/WeaponBarrelComponent.h"
#include "GameCodeTypes.h"
#include "Characters/GCBaseCharacter.h"
#include <Net/UnrealNetwork.h>

ARangeWeaponItem::ARangeWeaponItem()
{
	SetReplicates(true);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);

	EquippedRightArmSocketName = SocketCharacterWeapon;
	ReticleType = EReticleType::Default;
}

void ARangeWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARangeWeaponItem, CurrentAmmo);
}

void ARangeWeaponItem::StartFire()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ShotTimer))
	{
		return;
	}

	bIsFiring = true;
	MakeShot();
}

const FFireModeSettings& ARangeWeaponItem::GetCurrentFireModeSettings() const
{
	return FireModesSettings[CurrentFireModeIndex];
}

FFireModeSettings& ARangeWeaponItem::GetCurrentFireModeSettings_Mutable()
{
	return FireModesSettings[CurrentFireModeIndex];
}

void ARangeWeaponItem::NextFireModeSettings()
{
	CurrentFireModeIndex++;
	if (CurrentFireModeIndex == FireModesSettings.Num())
	{
		CurrentFireModeIndex = 0;
	}
}

bool ARangeWeaponItem::HasAlternativeFireMode() const
{
	return FireModesSettings.Num() > 1;
}

void ARangeWeaponItem::OnShotTimerElapsed()
{
	if (!bIsFiring)
	{
		return;
	}

	switch (GetCurrentFireModeSettings().WeaponFireMode)
	{
		case EWeaponFireMode::Single:
		{
			StopFire();
			break;
		}
		case EWeaponFireMode::FullAuto:
		{
			MakeShot();
		}
	}
}

void ARangeWeaponItem::MakeShot()
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CanShoot())
	{
		StopFire();
		if (GetAmmo() == 0 && GetCurrentFireModeSettings().bAutoReload)
		{
			CharacterOwner->Reload();
		}
		return;
	}

	EndReload(false);

	CharacterOwner->PlayAnimMontage(CharacterFireMontage);
	PlayAnimMontage(WeaponFireMontage);

	FVector ShotLocation;
	FRotator ShotRotation;

	if (CharacterOwner->IsPlayerControlled())
	{
		APlayerController* Controller = CharacterOwner->GetController<APlayerController>();
		if (!IsValid(Controller))
		{
			return;
		}

		Controller->GetPlayerViewPoint(ShotLocation, ShotRotation);
	}
	else
	{
		ShotLocation = WeaponBarrel->GetComponentLocation();
		ShotRotation = CharacterOwner->GetBaseAimRotation();
	}

	FVector ShotDirection = ShotRotation.RotateVector(FVector::ForwardVector);

	SetAmmo(GetAmmo() - 1);
	WeaponBarrel->Shot(ShotLocation, ShotDirection, GetCurrentBulletSpreadAngle());

	if (GetAmmo() == 0 && GetCurrentFireModeSettings().bAutoReloadAfterShot)
	{
		CharacterOwner->Reload();
	}

	GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ARangeWeaponItem::OnShotTimerElapsed, GetShotTimerInterval(), false);
}

void ARangeWeaponItem::StopFire()
{
	bIsFiring = false;
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetAmmo(CurrentAmmo);
	}
}


FTransform ARangeWeaponItem::GetForeGripTransform() const
{
	return WeaponMesh->GetSocketTransform(SocketWeaponForeGrip);
}

FTransform ARangeWeaponItem::GetBowStringTransform() const
{
	return WeaponMesh->GetSocketTransform(SocketBowString);
}

void ARangeWeaponItem::StartAim()
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (IsValid(CharacterOnStartAimingMontage) && IsValid(WeaponOnStartAimingMontage))
	{
		// bIsAiming will be set on aim notify
		float MontageDuration = CharacterOwner->PlayAnimMontage(CharacterOnStartAimingMontage);
		// GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]() { bIsAiming = true; }, MontageDuration, false);
	}
	else {
		bIsAiming = true;
	}
}

void ARangeWeaponItem::StopAim()
{
	bIsAiming = false;
}

int32 ARangeWeaponItem::GetAmmo() const
{
	return CurrentAmmo;
}

void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(GetAmmo());
	}
}

void ARangeWeaponItem::Server_SetAmmo_Implementation(int32 Ammo)
{
	SetAmmo(CurrentAmmo);
}

void ARangeWeaponItem::SetIsAiming(bool bIsAiming_New)
{
	bIsAiming = bIsAiming_New;
}

void ARangeWeaponItem::PlayWeaponStartAimingAnimMontage()
{
	if (IsValid(WeaponOnStartAimingMontage))
	{
		PlayAnimMontage(WeaponOnStartAimingMontage);
	}
}

bool ARangeWeaponItem::CanShoot() const
{
	return GetAmmo() > 0;
}

EAmmunitionType ARangeWeaponItem::GetAmmoType() const
{
	return GetCurrentFireModeSettings().AmmoType;
}

void ARangeWeaponItem::StartReload()
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	bIsReloading = true;
	if (IsValid(CharacterReloadMontage))
	{
		float MontageDuration = CharacterOwner->PlayAnimMontage(CharacterReloadMontage);
		PlayAnimMontage(WeaponReloadMontage);
		if (GetCurrentFireModeSettings().ReloadType == EReloadType::FullClip)
		{
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]() { EndReload(true); }, MontageDuration, false);
		}
	}
	else
	{
		EndReload(true);
	}
}

void ARangeWeaponItem::EndReload(bool bIsSuccess)
{
	if (!bIsReloading)
	{
		return;
	}

	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();

	if (!bIsSuccess)
	{
		if (IsValid(CharacterOwner))
		{
			CharacterOwner->StopAnimMontage(CharacterReloadMontage);
			StopAnimMontage(WeaponReloadMontage);
		}
	}

	if (GetCurrentFireModeSettings().ReloadType == EReloadType::ByBullet)
	{
		if (IsValid(CharacterOwner))
		{
			UAnimInstance* CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
			if (IsValid(CharacterAnimInstance))
			{
				CharacterAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, CharacterReloadMontage);
			}

			UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
			if (IsValid(WeaponAnimInstance))
			{
				WeaponAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, WeaponReloadMontage);
			}
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	bIsReloading = false;
	if (bIsSuccess && OnReloadComplete.IsBound())
	{
		OnReloadComplete.Broadcast();
	}

}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bIsAiming ? AimReticleType : ReticleType;
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(FireModesSettings.Num() > 0, TEXT("RangeWeaponItem should have at least one fire mode"));
	WeaponBarrel->SetFireModeSettings(FireModesSettings[0]);

	for (FFireModeSettings& CurrentSettings : FireModesSettings)
	{
		CurrentSettings.Ammo = CurrentSettings.MaxAmmo;
	}
}

float ARangeWeaponItem::PlayAnimMontage(UAnimMontage* AnimMontage)
{
	UAnimInstance* AnimInstance = WeaponMesh->GetAnimInstance();
	float Result = 0.0f;
	if (IsValid(AnimInstance))
	{
		Result = AnimInstance->Montage_Play(AnimMontage);;
	}
	return Result;
}

void ARangeWeaponItem::StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime /*= 0.0f*/)
{
	UAnimInstance* AnimInstance = WeaponMesh->GetAnimInstance();
	if (IsValid(AnimInstance))
	{
		AnimInstance->Montage_Stop(BlendOutTime, AnimMontage);;
	}
}

float ARangeWeaponItem::GetRateOfFire() const
{
	return GetCurrentFireModeSettings().RateOfFire;
}

float ARangeWeaponItem::GetShotTimerInterval() const
{
	return 60.0f / GetRateOfFire();
}

float ARangeWeaponItem::GetCurrentBulletSpreadAngle() const
{
	float AngleInDegrees = bIsAiming ? AimSpreadAngle : SpreadAngle;
	return FMath::DegreesToRadians(AngleInDegrees);
}

void ARangeWeaponItem::ChangeAmmoType()
{
	if (!HasAlternativeFireMode())
	{
		return;
	}

	// saving other fire mode current ammo amount
	GetCurrentFireModeSettings_Mutable().Ammo = CurrentAmmo;

	// updating fire mode
	NextFireModeSettings();
	WeaponBarrel->SetFireModeSettings(GetCurrentFireModeSettings());

	SetAmmo(GetCurrentFireModeSettings().Ammo);
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetAmmo(CurrentAmmo);
	}
}

int32 ARangeWeaponItem::GetMaxAmmo() const
{
	return GetCurrentFireModeSettings().MaxAmmo;
}

void ARangeWeaponItem::UnEquipWeapon()
{
	GetCurrentFireModeSettings_Mutable().Ammo = CurrentAmmo;
}

void ARangeWeaponItem::EquipWeapon()
{
	SetAmmo(GetCurrentFireModeSettings().Ammo);
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetAmmo(CurrentAmmo);
	}
}
