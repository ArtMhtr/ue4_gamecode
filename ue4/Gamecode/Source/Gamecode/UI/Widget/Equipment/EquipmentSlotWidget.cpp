// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSlotWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include <Components/Image.h>
#include <Components/TextBlock.h>
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "../Inventory/InventorySlotWidget.h"
#include "Inventory/InventoryItemFactory.h"

void UEquipmentSlotWidget::InitializeEquipmentSlot(TWeakObjectPtr<AEquipableItem> Equipment, int32 Index)
{
	LinkedEquipableItem = Equipment;
	SlotIndexInComponent = Index;
	AdapterLinkedInventorySlot = NewObject<UInventorySlot>(GetOuter());

	if (IsValid(AdapterLinkedInventorySlot) && LinkedEquipableItem.IsValid())
	{
		AdapterLinkedInventorySlot->Item = UInventoryItemFactory::CreateInventoryItem(GetOwningPlayer(), Equipment->GetDataTableID(), EInventoryItemType::Weapon);
		AdapterLinkedInventorySlot->Count = 1;
	}
}

void UEquipmentSlotWidget::UpdateView()
{
	if (LinkedEquipableItem.IsValid())
	{
		TBWeaponName->SetText(AdapterLinkedInventorySlot->Item->GetDescription().Name);
		ImageWeaponIcon->SetBrushFromTexture(AdapterLinkedInventorySlot->Item->GetDescription().Icon);
	}
	else
	{
		TBWeaponName->SetText(FText::FromName(NAME_None));
		ImageWeaponIcon->SetBrushFromTexture(nullptr);
	}
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!LinkedEquipableItem.IsValid())
	{
		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOpetaion)
{
	check(DragAndDropWidgetClass.Get());

	if (!IsValid(AdapterLinkedInventorySlot->Item))
	{
		return;
	}

	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), DragAndDropWidgetClass);
	DragWidget->SetItemIcon(AdapterLinkedInventorySlot->Item->GetDescription().Icon);
	DragWidget->SetItemsCount(AdapterLinkedInventorySlot->Count);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	DragOperation->Payload = CreatePayload(AdapterLinkedInventorySlot);
	OutOpetaion = DragOperation;

	AdapterLinkedInventorySlot->ClearSlot();
	LinkedEquipableItem.Reset();

	OnEquipmentRemoveFromSlot.ExecuteIfBound(SlotIndexInComponent);

	UpdateView();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion)
{
	bool bResult = false;
	const UInventorySlot* OperationObject = Cast<UInventorySlot>(InOpetaion->Payload);
	if (IsValid(OperationObject))
	{
		UWeaponInventoryItem* WeaponItem = Cast<UWeaponInventoryItem>(OperationObject->Item);
		if (IsValid(WeaponItem))
		{
			bResult = OnEquipmentDropInSlot.Execute(WeaponItem->GetEquipWeaponClass(), SlotIndexInComponent);
		}
	}

	DestroyPayload(InOpetaion->Payload);
	return bResult;
}

void UEquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion)
{
	const UInventorySlot* OperationObject = Cast<UInventorySlot>(InOpetaion->Payload);
	if (IsValid(OperationObject))
	{
		UWeaponInventoryItem* WeaponItem = Cast<UWeaponInventoryItem>(OperationObject->Item);
		if (IsValid(WeaponItem))
		{
			OnEquipmentDropInSlot.Execute(WeaponItem->GetEquipWeaponClass(), SlotIndexInComponent);
		}
	}

	DestroyPayload(InOpetaion->Payload);
}

UObject* UEquipmentSlotWidget::CreatePayload(const UInventorySlot* InventorySlot)
{
	return DuplicateObject<UInventorySlot>(InventorySlot, GetOwningPlayer());
}

void UEquipmentSlotWidget::DestroyPayload(UObject* Payload)
{
	if (Payload)
	{
		Payload->ConditionalBeginDestroy();
	}
}

void UEquipmentSlotWidget::InitializeInventorySlotWithPayload(UObject* Payload, OUT UInventorySlot& InventorySlot)
{
	UInventorySlot* PayloadSlot = Cast<UInventorySlot>(Payload);
	if (IsValid(PayloadSlot))
	{
		AdapterLinkedInventorySlot->Item = PayloadSlot->Item;
		AdapterLinkedInventorySlot->Count = PayloadSlot->Count;
		InventorySlot.UpdateSlotState();
	}
}
