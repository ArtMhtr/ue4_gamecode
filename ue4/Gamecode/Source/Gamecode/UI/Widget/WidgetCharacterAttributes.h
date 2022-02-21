// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCodeTypes.h"
#include "WidgetCharacterAttributes.generated.h"

UCLASS()
class GAMECODE_API UWidgetCharacterAttributes : public UUserWidget
{
	GENERATED_BODY()

public:	

	UFUNCTION()
	void UpdateHealthBarPercent();

	UFUNCTION()
	void UpdateOxygenBarPercent();

	UFUNCTION()
	void UpdateStaminaBarPercent();

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
	float HealthBarPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
	float StaminaBarPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
	float OxygenBarPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes | Visibility")
	ESlateVisibility StaminaBarVisibility = ESlateVisibility::Hidden;

	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes | Visibility")
	ESlateVisibility OxygenBarVisibility = ESlateVisibility::Hidden;

private:
	float GetBarPercent(ECharacterAttributeType CharacterAttributeType) const;
	bool IsBarFull(ECharacterAttributeType CharacterAttributeType) const;
};
