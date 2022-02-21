// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_EndSlide.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"

void UAnimNotify_EndSlide::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(CharacterOwner) && (CharacterOwner->GetNetMode() == NM_Standalone || CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy || (CharacterOwner->GetNetMode() == NM_ListenServer && CharacterOwner->GetLocalRole() == ROLE_Authority)))
	{
		CharacterOwner->GetBaseCharacterMovementComponent()->StopSlide();
	}
}
