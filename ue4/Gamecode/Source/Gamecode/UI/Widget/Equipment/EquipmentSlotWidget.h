// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventorySlot.h"
#include "EquipmentSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UInventorySlot;
class AEquipableItem;
class UWeaponInventoryItem;
class UInventorySlotWidget;

UCLASS()
class GAMECODE_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnEquipmentDropInSlot, const TSubclassOf<AEquipableItem>&, int32);
	DECLARE_DELEGATE_OneParam(FOnEquipmentRemoveFromSlot, int32);

	FOnEquipmentDropInSlot OnEquipmentDropInSlot;
	FOnEquipmentRemoveFromSlot OnEquipmentRemoveFromSlot;

	void InitializeEquipmentSlot(TWeakObjectPtr<AEquipableItem> Equipment, int32 Index);
	void UpdateView();

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ImageWeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TBWeaponName;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventorySlotWidget> DragAndDropWidgetClass;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOpetaion) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion) override;

private:
	TWeakObjectPtr<AEquipableItem> LinkedEquipableItem;

	UPROPERTY()
	UInventorySlot* AdapterLinkedInventorySlot;
	int32 SlotIndexInComponent;

	UObject* CreatePayload(const UInventorySlot* InventorySlot);
	void DestroyPayload(UObject* Payload);
	void InitializeInventorySlotWithPayload(UObject* Payload, OUT UInventorySlot& InventorySlot);
};
