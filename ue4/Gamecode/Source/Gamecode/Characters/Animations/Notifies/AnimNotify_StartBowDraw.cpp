// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_StartBowDraw.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"

void UAnimNotify_StartBowDraw::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(CharacterOwner))
	{
		ARangeWeaponItem* CurrentWeaponItem = CharacterOwner->GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();;
		if (IsValid(CurrentWeaponItem))
		{
			CurrentWeaponItem->PlayWeaponStartAimingAnimMontage();
			CurrentWeaponItem->SetIsAiming(true);
		}
	}
}
