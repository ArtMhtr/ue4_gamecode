// Fill out your copyright notice in the Description page of Project Settings.


#include "Adrenaline.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Characters/GCBaseCharacter.h"

bool UAdrenaline::Consume(AGCBaseCharacter* ConsumeTarget)
{
	ConsumeTarget->GetCharacterAttributesComponent_Mutable()->RestoreFullStamina();
	return true;
}
