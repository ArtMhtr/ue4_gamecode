// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "WeaponBarrelComponent.generated.h"

class UNiagaraSystem;
struct FFireModeSettings;

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo() : Location_Mul_10(FVector_NetQuantize100::ZeroVector), Direction(FVector_NetQuantizeNormal::ZeroVector) {};
	FShotInfo(FVector Location_In, FVector Direction_In) : Location_Mul_10(Location_In * 10.0f), Direction(Direction_In) {};

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;
	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	FVector_NetQuantize100 GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector_NetQuantizeNormal GetDirection() const { return Direction; }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UWeaponBarrelComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponBarrelComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Shot(FVector ShortStart, FVector ShotDirection, float SpreadAngle);
	void SetFireModeSettings(const FFireModeSettings& NewFireModeSettings);

private:
	void ShotInternal(const TArray<FShotInfo>& ShotsInfo);

	UFUNCTION(Server, Reliable)
	void Server_Shot(const TArray<FShotInfo>& ShotsInfo);

	UPROPERTY(ReplicatedUsing=OnRep_LastShotsInfo)
	TArray<FShotInfo> LastShotsInfo;

	UPROPERTY(Replicated)
	TArray<AGCProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
	int32 CurrentProjectileIndex = 0;

	UFUNCTION()
	void OnRep_LastShotsInfo();

	FFireModeSettings CurrentFireModeSettings;
	FVector GetBulletSpreadOffset(float Angle, FRotator ShotRotation);

	UFUNCTION()
	void ProcessProjectileHit(AGCProjectile* Projectile, const FVector& ShotStartLocation, const FHitResult& Hit, const FVector& Direction);

	UFUNCTION()
	void ProcessHit(const FVector& ShotStartLocation, const FHitResult& Hit, const FVector& Direction);

	void FillProjectilePool(const FFireModeSettings& FireModeSettings);
	bool HitScan(FVector ShortStart, OUT FVector& ShotEnd, FVector ShotDirection);
	void LaunchProjectile(FVector LaunchStart, FVector ShotDirection);

	APawn* GetOwningPawn() const;
	AController* GetController() const;

	const FVector ProjectilePoolLocation = FVector(0.0f, 0.0f, -100);
};
