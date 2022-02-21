// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCodeTypes.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedEvent, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnOxygenChanged, float);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterAttributesComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnDeathEventSignature OnDeathEvent;
	FOutOfStaminaEventSignature OnStaminaChangedState;

	FOnStaminaChangedEvent OnStaminaChangedEvent;
	FOnHealthChanged OnHealthChangedEvent;
	FOnOxygenChanged OnOxygenChanged;

	bool IsAlive() const { return Health > 0; }
	float GetStamina() const { return Stamina; }
	float GetAttributePercent(ECharacterAttributeType CharacterAttributeType) const;

	void AddHealth(float HealthToAdd);
	void RestoreFullStamina();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/* Stamina */
	void UpdateStaminaValue(float DeltaTime);

	void OnStaminaChanged();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float StaminaRestoreVelocity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float SprintStaminaConsumptionVelocity = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float MaxOxygen = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OxygenRestoreVelocity = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float SwimOxygenConsumptionVelocity = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OxygenLackDamage = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OxygenLackDamagePeriod = 2.0f; // in seconds

private:

	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 0.0f;

	UFUNCTION()
	void OnRep_Health();

	void OnHealthChanged();

	float Stamina = 0.0f;

	void SpendStamina(float DeltaSeconds);
	void RestoreStamina(float DeltaSeconds);
	float Oxygen = 0.0f;

	void UpdateOxygenValue(float DeltaTime);
	void OnDamageTimerElapsed();
	FTimerHandle OxygenDamageTimer;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacterOwner;
};
