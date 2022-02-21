// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UInventorySlot;
class UImage;
class UTextBlock;

UCLASS()
class GAMECODE_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeItemSlot(UInventorySlot& InventorySlot);
	void UpdateView();
	void SetItemIcon(UTexture2D* Icon);
	void SetItemsCount(uint32 Count);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ImageItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemsCount;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOpetaion) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDtopEvent, UDragDropOperation* InOpetaion) override;

private:
	bool IsValidLinkedSlot() const;

	UObject* CreatePayload(const UInventorySlot* InventorySlot);
	void DestroyPayload(UObject* Payload);
	void InitializeInventorySlotWithPayload(UObject* Payload, OUT UInventorySlot& InventorySlot);

	TWeakObjectPtr<UInventorySlot> LinkedSlot;
};
