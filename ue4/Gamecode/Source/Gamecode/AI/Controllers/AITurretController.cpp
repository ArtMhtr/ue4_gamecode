// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"
#include "AI/Characters/Turret.h"
#include <Perception/AISense_Sight.h>
#include <Perception/AIPerceptionComponent.h>
#include <Perception/AISense_Damage.h>

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController::SetPawn can only be used with ATurret"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if (!CachedTurret.IsValid())
	{
		return;
	}

	AActor* ClosestActor = GetClosestSensedActor(UAISense_Damage::StaticClass());
	if (!ClosestActor)
	{
		ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	}

	CachedTurret->CurrentTarget = ClosestActor;
	CachedTurret->OnCurrentTargetSet();
}
