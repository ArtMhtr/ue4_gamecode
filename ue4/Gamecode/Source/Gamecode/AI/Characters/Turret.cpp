// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"
#include "Components/Weapon/WeaponBarrelComponent.h"
#include <AIController.h>
#include <Perception/AISense_Damage.h>
#include "Subsystems/DebugSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include <DrawDebugHelpers.h>
#include <Net/UnrealNetwork.h>

// Sets default values
ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBase"));
	TurretBaseComponent->SetupAttachment(TurretRoot);

	TurretBarrelComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBarrel"));
	TurretBarrelComponent->SetupAttachment(TurretBaseComponent);

	WeaponBarrel = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(TurretBarrelComponent);

	SetReplicates(true);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	WeaponBarrel->SetFireModeSettings(FireModeSettings);
	Health = MaxHealth;
	OnTakeAnyDamage.AddDynamic(this, &ATurret::OnTakeAnyDamageEvent);
	OnTurretDestroyed.AddDynamic(this, &ATurret::OnDeath);
}

void ATurret::OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	UAISense_Damage::ReportDamageEvent(GetWorld(), DamagedActor, InstigatedBy->GetPawn(), Damage, InstigatedBy->GetPawn()->GetActorLocation(), DamagedActor->GetActorLocation());
	Health -= Damage;
	Health = FMath::Clamp(Health, 0.0f, MaxHealth);

	if (FMath::IsNearlyZero(Health))
	{
		if (OnTurretDestroyed.IsBound())
		{
			OnTurretDestroyed.Broadcast();
		}
	}
}

void ATurret::OnRep_CurrentTarget()
{
	OnCurrentTargetSet();
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif

	switch (CurrentTurretState)
	{
		case ETurretState::Searching:
		{
			SearchingMovement(DeltaTime);
			break;
		}
		case ETurretState::Firing:
		{
			FiringMovement(DeltaTime);
			break;
		}
	}
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATurret, CurrentTarget);

}

void ATurret::OnCurrentTargetSet()
{
	ETurretState NewState = IsValid(CurrentTarget) ? ETurretState::Firing : ETurretState::Searching;
	SetCurrentTurretState(NewState);
}

FVector ATurret::GetPawnViewLocation() const
{
	return WeaponBarrel->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return WeaponBarrel->GetComponentRotation();
}

void ATurret::OnDeath()
{
	SetActorTickEnabled(false);
	SetCanBeDamaged(false);
}

void ATurret::MakeShot()
{
	FVector ShotLocation = WeaponBarrel->GetComponentLocation();
	FVector ShotDirection = WeaponBarrel->GetComponentRotation().RotateVector(FVector::ForwardVector);
	float SpreadAngle = FMath::DegreesToRadians(BulletSpreadAngle);
	WeaponBarrel->Shot(ShotLocation, ShotDirection, SpreadAngle);
}

void ATurret::SetCurrentTurretState(ETurretState NewState)
{
	bool bIsStateChanged = (NewState != CurrentTurretState);
	CurrentTurretState = NewState;

	if (!bIsStateChanged)
	{
		return;
	}

	switch (CurrentTurretState)
	{
	case ETurretState::Firing:
	{
		GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ATurret::MakeShot, GetFireInterval(), true, FireDelayTime);
		break;
	}
	case ETurretState::Searching:
	{
		GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
		break;
	}
	}
}

float ATurret::GetFireInterval() const
{
	return 60.0f / FireModeSettings.RateOfFire;
}

void ATurret::SearchingMovement(float DeltaTime)
{
	FRotator TurretBaseRotation = TurretBaseComponent->GetRelativeRotation();
	TurretBaseRotation.Yaw += DeltaTime * BaseSearchingRotationRate;
	TurretBaseComponent->SetRelativeRotation(TurretBaseRotation);

	FRotator TurretBarrelRotation = TurretBarrelComponent->GetRelativeRotation();
	TurretBarrelRotation.Pitch = FMath::FInterpTo(TurretBarrelRotation.Pitch, 0.0f, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(TurretBarrelRotation);
}

void ATurret::FiringMovement(float DeltaTime)
{
	FVector BaseLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBaseComponent->GetComponentLocation()).GetSafeNormal2D();
	FQuat LookAtQuat = BaseLookAtDirection.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, BaseFiringInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	FVector BarrelLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBarrelComponent->GetComponentLocation()).GetSafeNormal();
	float BarrelLookAtPitchAngle = BarrelLookAtDirection.ToOrientationRotator().Pitch;

	FRotator BarrelLocalRotation = TurretBarrelComponent->GetRelativeRotation();
	BarrelLocalRotation.Pitch = FMath::FInterpTo(BarrelLocalRotation.Pitch, BarrelLookAtPitchAngle, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(BarrelLocalRotation);
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void ATurret::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	const float HealthTextOffset = 40.0f;
	FVector HealthTextLocation = GetOwner()->GetActorLocation() + HealthTextOffset * FVector::UpVector;
	DrawDebugString(GetWorld(), HealthTextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Green, 0.0f, true);
}
#endif
