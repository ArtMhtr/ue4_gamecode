// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SetMeleeHitEnabled.h"
#include "Characters/GCBaseCharacter.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"

void UAnimNotify_SetMeleeHitEnabled::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	AMeleeWeaponItem* MeleeWeapon = CharacterOwner->GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(MeleeWeapon))
	{
		MeleeWeapon->SetIsHitRegistrationEnabled(bIsHitRegistrationEnabled);
	}
}
