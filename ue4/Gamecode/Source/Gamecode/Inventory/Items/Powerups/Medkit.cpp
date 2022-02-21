// Fill out your copyright notice in the Description page of Project Settings.


#include "Medkit.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Characters/GCBaseCharacter.h"

bool UMedkit::Consume(AGCBaseCharacter* ConsumeTarget)
{
	ConsumeTarget->GetCharacterAttributesComponent_Mutable()->AddHealth(Health);
	return true;
}
