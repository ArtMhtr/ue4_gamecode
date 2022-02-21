// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UReticleWidget;
class UAmmoWidget;
class UWidgetCharacterAttributes;
class UHighlightInteractable;

UCLASS()
class GAMECODE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UReticleWidget* GetReticleWidget();
	UAmmoWidget* GetAmmoWidget();
	UWidgetCharacterAttributes* GetCharacterAttributesWidget();

	void SetHighlightInteractableVisibility(bool bIsVisible);
	void SetHighlightInteractableActionText(FName KeyName);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName AmmoWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName CharacterAttributesWidgetName;

	UPROPERTY(meta = (BindWidget))
	UHighlightInteractable* InteractableKey;
};
