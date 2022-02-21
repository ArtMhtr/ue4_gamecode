// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ThrowGrenade.h"
#include <AIController.h>
#include <BehaviorTree/BlackboardComponent.h>
#include "Characters/GCBaseCharacter.h"

UBTService_ThrowGrenade::UBTService_ThrowGrenade()
{
	NodeName = "ThrowGrenade";
}

void UBTService_ThrowGrenade::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackBoard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(AIController) || !IsValid(BlackBoard))
	{
		return;
	}

	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(AIController->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	const UCharacterEquipmentComponent* EquipmentComponent = Character->GetCharacterEquipmentComponent();
	if (!EquipmentComponent->CanThrowItem())
	{
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BlackBoard->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!IsValid(CurrentTarget))
	{
		return;
	}

	float DistanceSq = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if (!FMath::IsWithin(DistanceSq, FMath::Square(MinThrowGrenadeDistance), FMath::Square(MaxThrowGrenadeDistance)))
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(ThrowGrenadeIntervalTimer))
	{
		return;
	}

	Character->EquipPrimaryItem();
	GetWorld()->GetTimerManager().SetTimer(ThrowGrenadeIntervalTimer, this, &UBTService_ThrowGrenade::OnThrowGrenadeIntervalTimerElapsed, MaxGrenadeThrowInterval, false);
}

void UBTService_ThrowGrenade::OnThrowGrenadeIntervalTimerElapsed()
{
	GetWorld()->GetTimerManager().ClearTimer(ThrowGrenadeIntervalTimer);
}
