// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "GameCodeTypes.h"
#include "MeleeWeaponItem.generated.h"

class UDamageType;
class UAnimMontage;
class UMeleeHitRegistrator;

USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack")
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	UAnimMontage* AttackMontage;
};

UCLASS(Blueprintable)
class GAMECODE_API AMeleeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()
	
public:

	AMeleeWeaponItem();
	virtual void BeginPlay() override;
	void StartAttack(EMeleeAttackType AttackType);
	void SetIsHitRegistrationEnabled(bool bIsHitRegistrationEnabled);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack")
	TMap<EMeleeAttackType, FMeleeAttackDescription> Attacks;

private:
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& HitDirection);

	TArray<UMeleeHitRegistrator*> HitRegistrators;
	TSet<AActor*> HitActors;

	void OnAttackTimerElapsed();
	FTimerHandle AttackTimer;
	FMeleeAttackDescription* CurrentAttack;
};
