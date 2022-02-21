// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include <Components/Image.h>
#include "Characters/GCBaseCharacter.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include <Components/TextBlock.h>
#include "Inventory/InventorySlot.h"

void UInventorySlotWidget::InitializeItemSlot(UInventorySlot& InventorySlot)
{
	LinkedSlot = &InventorySlot;

	UInventorySlot::FInventorySlotUpdate OnInventorySlotUpdate;
	OnInventorySlotUpdate.BindUObject(this, &UInventorySlotWidget::UpdateView);
	LinkedSlot->BindOnInventorySlotUpdate(OnInventorySlotUpdate);
}

void UInventorySlotWidget::UpdateView()
{
	if (IsValidLinkedSlot())
	{
		ImageItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetDescription().Icon);
		ItemsCount->SetVisibility(ESlateVisibility::Visible);
		ItemsCount->SetText(FText::AsNumber(LinkedSlot->Count));
	}
	else
	{
		ImageItemIcon->SetBrushFromTexture(nullptr);
		ItemsCount->SetVisibility(ESlateVisibility::Hidden);
		ItemsCount->SetText(FText::GetEmpty());
	}
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsValidLinkedSlot())
	{
		return FReply::Handled();
	}

	FKey MouseButton = InMouseEvent.GetEffectingButton();
	if (MouseButton == EKeys::RightMouseButton)
	{
		TWeakObjectPtr<UInventoryItem> LinkedSlotItem = LinkedSlot->Item;
		AGCBaseCharacter* ItemOwner = Cast<AGCBaseCharacter>(LinkedSlotItem->GetOuter());

		if (LinkedSlotItem->IsConsumable())
		{
			LinkedSlotItem->Consume(ItemOwner);
			ItemOwner->RemoveItems(LinkedSlotItem->GetItemId(), LinkedSlotItem->GetItemType(), 1);
		}

		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOpetaion)
{
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), GetClass());
	DragWidget->ImageItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetDescription().Icon);
	DragWidget->ItemsCount->SetText(FText::AsNumber(LinkedSlot->Count));

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = CreatePayload(LinkedSlot.Get());
	OutOpetaion = DragOperation;

	LinkedSlot->ClearSlot();
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion)
{
	if (!IsValidLinkedSlot())
	{
		InitializeInventorySlotWithPayload(InOpetaion->Payload, *LinkedSlot);
		DestroyPayload(InOpetaion->Payload);
		return true;
	}

	return false;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion)
{
	InitializeInventorySlotWithPayload(InOpetaion->Payload, *LinkedSlot);

	AGCBaseCharacter* ItemOwner = Cast<AGCBaseCharacter>(LinkedSlot->GetOuter());
	if (IsValid(ItemOwner))
	{
		ItemOwner->DropItems(LinkedSlot.Get());
	}
	DestroyPayload(InOpetaion->Payload);
}

bool UInventorySlotWidget::IsValidLinkedSlot() const
{
	return LinkedSlot.Get() && IsValid(LinkedSlot->Item) && LinkedSlot->Count;
}

UObject* UInventorySlotWidget::CreatePayload(const UInventorySlot* InventorySlot)
{
	return DuplicateObject<UInventorySlot>(InventorySlot, GetOwningPlayer());
}

void UInventorySlotWidget::DestroyPayload(UObject* Payload)
{
	if (Payload)
	{
		Payload->ConditionalBeginDestroy();
	}
}

void UInventorySlotWidget::InitializeInventorySlotWithPayload(UObject* Payload, OUT UInventorySlot& InventorySlot)
{
	UInventorySlot* PayloadSlot = Cast<UInventorySlot>(Payload);
	if (IsValid(PayloadSlot))
	{
		InventorySlot.Item = PayloadSlot->Item;
		InventorySlot.Count = PayloadSlot->Count;
		InventorySlot.UpdateSlotState();
	}
}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon)
{
	ImageItemIcon->SetBrushFromTexture(Icon);
}

void UInventorySlotWidget::SetItemsCount(uint32 Count)
{
	ItemsCount->SetText(FText::AsNumber(Count));
}

