// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Turret.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurretDeathEventSignature);

class UWeaponBarrelComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ETurretState : uint8
{
	Searching, 
	Firing
};

UCLASS()
class GAMECODE_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnCurrentTargetSet();

	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;

	UPROPERTY(BlueprintAssignable)
	FOnTurretDeathEventSignature OnTurretDestroyed;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTarget)
	AActor* CurrentTarget = nullptr;

	UFUNCTION()
	void OnDeath();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBarrelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarrelComponent* WeaponBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float BaseSearchingRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float BarrelPitchRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float BaseFiringInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = -60.0f, ClampMin = -60.0f, UIMax = 60.0f, ClampMax = 60.0f))
	float MaxBarrelPitchAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = -60.0f, ClampMin = -60.0f, UIMax = 60.0f, ClampMax = 60.0f))
	float MinBarrelPitchAngle = -30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float BulletSpreadAngle = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float FireDelayTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Team", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters | Fire")
	FFireModeSettings FireModeSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret parameters", meta = (UIMin = 1.0f, ClampMin = 1.0f))
	float MaxHealth = 100.0f;

private:
	ETurretState CurrentTurretState = ETurretState::Searching;
	float Health = 0.0f;

	void MakeShot();
	void SetCurrentTurretState(ETurretState NewState);
	float GetFireInterval() const;
	void SearchingMovement(float DeltaTime);
	void FiringMovement(float DeltaTime);
	FTimerHandle ShotTimer;

	UFUNCTION()
	void OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_CurrentTarget();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif
};
