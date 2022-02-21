#include "PlayerHUDWidget.h"
#include "Characters/GCBaseCharacter.h"
#include "ReticleWidget.h"
#include <Blueprint/WidgetTree.h>
#include "AmmoWidget.h"
#include "WidgetCharacterAttributes.h"
#include "HighlightInteractable.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget()
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget()
{
	return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

UWidgetCharacterAttributes* UPlayerHUDWidget::GetCharacterAttributesWidget()
{
	return WidgetTree->FindWidget<UWidgetCharacterAttributes>(CharacterAttributesWidgetName);
}

void UPlayerHUDWidget::SetHighlightInteractableVisibility(bool bIsVisible)
{
	if (IsValid(InteractableKey))
	{
		if (bIsVisible)
		{
			InteractableKey->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			InteractableKey->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UPlayerHUDWidget::SetHighlightInteractableActionText(FName KeyName)
{
	if (IsValid(InteractableKey))
	{
		InteractableKey->SetActionText(KeyName);
	}
}
