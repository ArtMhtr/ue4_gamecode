// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentViewWidget.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "EquipmentSlotWidget.h"
#include <Components/VerticalBox.h>

void UEquipmentViewWidget::InitializeEquipmentWidget(UCharacterEquipmentComponent* CharacterEquipmentComponent)
{
	LinkedEquipmentComponent = CharacterEquipmentComponent;
	const TArray<AEquipableItem*> Items = LinkedEquipmentComponent->GetItems();

	for (int32 Index = 1; Index < Items.Num(); ++Index)
	{
		if (IsEquipmentWidgetVisibleSlot((EEquipmentSlots)Index))
		{
			AddEquipmentSlotView(Items[Index], Index);
		}
	}
}

void UEquipmentViewWidget::AddEquipmentSlotView(AEquipableItem* LinkToWeapon, int32 SlotIndex)
{
	check(IsValid(DefaultSlotViewClass.Get()));

	UEquipmentSlotWidget* SlotWidget = CreateWidget<UEquipmentSlotWidget>(this, DefaultSlotViewClass);
	if (IsValid(SlotWidget))
	{
		SlotWidget->InitializeEquipmentSlot(LinkToWeapon, SlotIndex);

		VBWeaponSlot->AddChildToVerticalBox(SlotWidget);
		SlotWidget->UpdateView();
		SlotWidget->OnEquipmentDropInSlot.BindUObject(this, &UEquipmentViewWidget::EquipEquipmentToSlot);
		SlotWidget->OnEquipmentRemoveFromSlot.BindUObject(this, &UEquipmentViewWidget::RemoveEquipmentFromSlot);
	}
}

void UEquipmentViewWidget::UpdateSlot(int32 SlotIndex)
{
	UEquipmentSlotWidget* WidgetToUpdate = Cast<UEquipmentSlotWidget>(VBWeaponSlot->GetChildAt(SlotIndex - 1));
	if (IsValid(WidgetToUpdate))
	{
		WidgetToUpdate->InitializeEquipmentSlot(LinkedEquipmentComponent->GetItems()[SlotIndex], SlotIndex);
		WidgetToUpdate->UpdateView();
	}
}

bool UEquipmentViewWidget::EquipEquipmentToSlot(const TSubclassOf<AEquipableItem>& WeaponClass, int32 SenderIndex)
{
	const bool Result = LinkedEquipmentComponent->AddEquipmentItemToSlot(WeaponClass, SenderIndex);
	if (Result)
	{
		UpdateSlot(SenderIndex);
	}
	return Result;
}

void UEquipmentViewWidget::RemoveEquipmentFromSlot(int32 SlotIndex)
{
	LinkedEquipmentComponent->RemoveItemFromSlot(SlotIndex);
}

bool UEquipmentViewWidget::IsEquipmentWidgetVisibleSlot(EEquipmentSlots EquipmentSlot) const
{
	switch (EquipmentSlot)
	{
		case EEquipmentSlots::PrimaryWeapon:
		case EEquipmentSlots::SecondaryWeapon:
		case EEquipmentSlots::SideArm:
			return true;
		default:
			return false;
	}
}
