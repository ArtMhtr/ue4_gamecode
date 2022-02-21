// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "Subsystems/DebugSubsystem.h"
#include "Characters/GCBaseCharacter.h"
#include "Subsystems/DebugSubsystem.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "Subsystems/DebugSubsystem.h"
#include <Components/CapsuleComponent.h>
#include <Kismet/GameplayStatics.h>
#include <DrawDebugHelpers.h>
#include <Net/UnrealNetwork.h>
#include <Math/UnrealMathUtility.h>

UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


void UCharacterAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributesComponent, Health);
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay() - only GCBaseCharacter can own this component"));
	CachedBaseCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());

	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;

	if (GetOwner()->HasAuthority())
	{
		CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnTakeAnyDamage);
	}
}

void UCharacterAttributesComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributesComponent::OnHealthChanged()
{
	if (Health <= 0.0f)
	{
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}

	if (OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(Health);
	}
}

void UCharacterAttributesComponent::SpendStamina(float DeltaSeconds)
{
	Stamina -= SprintStaminaConsumptionVelocity * DeltaSeconds;
	Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	OnStaminaChanged();

	if (FMath::IsNearlyZero(Stamina))
	{
		OnStaminaChangedState.ExecuteIfBound(true);
	}
}

void UCharacterAttributesComponent::RestoreStamina(float DeltaSeconds)
{
	Stamina += StaminaRestoreVelocity * DeltaSeconds;
	Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	OnStaminaChanged();

	if (FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		OnStaminaChangedState.ExecuteIfBound(false);
	}
}

void UCharacterAttributesComponent::RestoreFullStamina()
{
	Stamina = MaxStamina;
	OnStaminaChanged();

	if (FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		OnStaminaChangedState.ExecuteIfBound(false);
	}
}


void UCharacterAttributesComponent::UpdateStaminaValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting())
	{
		SpendStamina(DeltaTime);
	}
	else
	{
		RestoreStamina(DeltaTime);
	}
}

void UCharacterAttributesComponent::OnStaminaChanged()
{
	if (OnStaminaChangedEvent.IsBound())
	{
		OnStaminaChangedEvent.Broadcast(Stamina);
	}
}

float UCharacterAttributesComponent::GetAttributePercent(ECharacterAttributeType CharacterAttributeType) const
{
	switch (CharacterAttributeType)
	{
	case ECharacterAttributeType::Health:
		return Health / MaxHealth;
	case ECharacterAttributeType::Oxygen:
		return Oxygen / MaxOxygen;
	case ECharacterAttributeType::Stamina:
		return Stamina / MaxStamina;
	default:
		return 0.0f;
	}
}

void UCharacterAttributesComponent::AddHealth(float HealthToAdd)
{
	Health = FMath::Clamp(Health + HealthToAdd, 0.0f, MaxHealth);
	OnHealthChanged();
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributesComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	const float HealthTextOffset = 10.0f;
	const float StaminaTextOffset = 1.0f;
	const float OxygenTextOffset = 20.0f;

	FVector HealthTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + HealthTextOffset) * FVector::UpVector;
	DrawDebugString(GetWorld(), HealthTextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Green, 0.0f, true);

	FVector StaminaTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + StaminaTextOffset) * FVector::UpVector;
	DrawDebugString(GetWorld(), StaminaTextLocation, FString::Printf(TEXT("Stamina: %.2f"), Stamina), nullptr, FColor::Blue, 0.0f, true);

	FVector OxygenTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OxygenTextOffset) * FVector::UpVector;
	DrawDebugString(GetWorld(), OxygenTextLocation, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr, FColor::Cyan, 0.0f, true);
}
#endif

void UCharacterAttributesComponent::OnDamageTimerElapsed()
{
	CachedBaseCharacterOwner->TakeDamage(OxygenLackDamage, FDamageEvent(), CachedBaseCharacterOwner->GetController(), CachedBaseCharacterOwner.Get());
}

void UCharacterAttributesComponent::UpdateOxygenValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater())
	{
		Oxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
	}
	else
	{
		Oxygen += OxygenRestoreVelocity * DeltaTime;
	}

	Oxygen = FMath::Clamp(Oxygen, 0.0f, MaxOxygen);
	if (FMath::IsNearlyZero(Oxygen))
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(OxygenDamageTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(OxygenDamageTimer, this, &UCharacterAttributesComponent::OnDamageTimerElapsed, OxygenLackDamagePeriod, false);
		}
	}
	else if (GetWorld()->GetTimerManager().IsTimerActive(OxygenDamageTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(OxygenDamageTimer);
	}

	if (OnOxygenChanged.IsBound())
	{
		OnOxygenChanged.Broadcast(Oxygen);
	}
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	OnHealthChanged();
}

void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStaminaValue(DeltaTime);
	UpdateOxygenValue(DeltaTime);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif

}

