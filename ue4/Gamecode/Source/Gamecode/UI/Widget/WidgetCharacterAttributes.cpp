// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetCharacterAttributes.h"
#include "Characters/GCBaseCharacter.h"
#include <Widgets/Views/STableViewBase.h>

float UWidgetCharacterAttributes::GetBarPercent(ECharacterAttributeType CharacterAttributeType) const
{
	float Result = 1.0f;
	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Pawn);
	if (IsValid(Character))
	{
		const UCharacterAttributesComponent* Attributes = Character->GetCharacterAttributesComponent();
		if (IsValid(Attributes))
		{
			Result = Attributes->GetAttributePercent(CharacterAttributeType);
		}
	}

	return Result;
}

bool UWidgetCharacterAttributes::IsBarFull(ECharacterAttributeType CharacterAttributeType) const
{
	return FMath::IsNearlyEqual(GetBarPercent(CharacterAttributeType), 1.0f);
}

void UWidgetCharacterAttributes::UpdateHealthBarPercent()
{
	HealthBarPercent = GetBarPercent(ECharacterAttributeType::Health);
}

void UWidgetCharacterAttributes::UpdateStaminaBarPercent()
{
	StaminaBarPercent = GetBarPercent(ECharacterAttributeType::Stamina);
	StaminaBarVisibility = IsBarFull(ECharacterAttributeType::Stamina) ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
}

void UWidgetCharacterAttributes::UpdateOxygenBarPercent()
{
	OxygenBarPercent = GetBarPercent(ECharacterAttributeType::Oxygen);
	OxygenBarVisibility = IsBarFull(ECharacterAttributeType::Oxygen) ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
}
