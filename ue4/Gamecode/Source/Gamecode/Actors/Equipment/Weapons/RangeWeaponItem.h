// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "GameCodeTypes.h"
#include "RangeWeaponItem.generated.h"

class UAnimMontage;
class UMaterialInstance;
class UNiagaraSystem;

UENUM()
enum class EWeaponFireMode : uint8
{
	Single = 0,
	FullAuto
};

UENUM()
enum class EReloadType : uint8
{
	FullClip = 0,
	ByBullet
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	UMaterialInstance* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	FVector DecalSize = FVector(5.0f, 5.0f, 5.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	float DecalLifeTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	float DecalFadeOutTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FFireModeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::HitScan"))
	float FiringRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	float DamageAmount = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (ClampMin = 1, UIMin = 1))
	int32 BulletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	EHitRegistrationType HitRegistrationType = EHitRegistrationType::HitScan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	TSubclassOf<class AGCProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::HitScan"))
	UCurveFloat* DamageFalloffDiagram = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings | VFX")
	UNiagaraSystem* MuzzleFlashFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings | VFX")
	UNiagaraSystem* TraceFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings | Decals")
	FDecalInfo DefaultDecalInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	EWeaponFireMode WeaponFireMode = EWeaponFireMode::Single;

	// Rate of fire in round per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (ClampMin = 1, UIMin = 1))
	int32 MaxAmmo = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	EAmmunitionType AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	bool bAutoReload = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings")
	bool bAutoReloadAfterShot = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Mode Settings", meta = (UIMin = 1, ClampMin = 1, EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	int32 ProjectilePoolSize = 10;

	int32 Ammo = 0;
};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32);
DECLARE_MULTICAST_DELEGATE(FOnReloadComplete);

UCLASS(Blueprintable)
class GAMECODE_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()
	
public:
	ARangeWeaponItem();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UnEquipWeapon();
	void EquipWeapon();

	void StartFire();
	void StopFire();

	FTransform GetForeGripTransform() const;
	FTransform GetBowStringTransform() const;

	void StartAim();
	void StopAim();

	float GetAimFOV() const { return AimFOV; }
	float GetAimMovementMaxSpeed() const { return AimMovementMaxSpeed; }
	int32 GetAmmo() const;
	void SetAmmo(int32 NewAmmo);
	bool CanShoot() const;
	int32 GetMaxAmmo() const;

	FOnAmmoChanged OnAmmoChanged;
	EAmmunitionType GetAmmoType() const;

	void StartReload();
	void EndReload(bool bIsSuccess);
	FOnReloadComplete OnReloadComplete;

	float GetChangeAimFOVDuration() const { return ChangeAimFOVDuration; }
	virtual EReticleType GetReticleType() const override;

	float GetAimTurnModifier() const { return AimTurnModifier; }
	float GetAimLookUpModifier() const { return AimLookUpModifier; }

	bool IsFiring() const { return bIsFiring; }
	bool IsReloading() const { return bIsReloading; }
	bool IsAiming() const { return bIsAiming; }

	void ChangeAmmoType();

	UFUNCTION(Server, Reliable)
	void Server_SetAmmo(int32 Ammo);

	void SetIsAiming(bool bIsAiming_New);
	void PlayWeaponStartAimingAnimMontage();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarrelComponent* WeaponBarrel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Weapon")
	UAnimMontage* WeaponReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* CharacterOnStartAimingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations | Character")
	UAnimMontage* WeaponOnStartAimingMontage;

	// Bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float SpreadAngle = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float AimSpreadAngle = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float AimMovementMaxSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 120.0f, UIMax = 120.0f))
	float AimFOV = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType AimReticleType = EReticleType::Default;

	// duration of FOV change in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ChangeAimFOVDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimTurnModifier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimLookUpModifier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Fire Modes")
	TArray<FFireModeSettings> FireModesSettings;

private:

	UPROPERTY(Replicated)
	int32 CurrentAmmo = 0;

	bool bIsReloading = false;
	bool bIsFiring = false;
	bool bIsAiming;

	uint32 CurrentFireModeIndex = 0;
	const FFireModeSettings& GetCurrentFireModeSettings() const;
	FFireModeSettings& GetCurrentFireModeSettings_Mutable();
	void NextFireModeSettings();
	bool HasAlternativeFireMode() const;

	void OnShotTimerElapsed();

	void MakeShot();
	FTimerHandle ShotTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle StartAimingTimer;

	float PlayAnimMontage(UAnimMontage* AnimMontage);
	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.0f);
	float GetRateOfFire() const;
	float GetShotTimerInterval() const;
	float GetCurrentBulletSpreadAngle() const;
};
