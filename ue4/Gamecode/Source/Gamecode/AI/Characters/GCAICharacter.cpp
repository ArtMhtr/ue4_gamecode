// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAICharacter.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include <AIController.h>


AGCAICharacter::AGCAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIPatrollingComponent = CreateDefaultSubobject<UAIPatrollingComponent>(TEXT("AIPatrollingComponent"));
}

UAIPatrollingComponent* AGCAICharacter::GetAIPatrollingComponent() const
{
	return AIPatrollingComponent;
}

UBehaviorTree* AGCAICharacter::GetBehaviorTree() const
{
	return BehaviorTree;
}
