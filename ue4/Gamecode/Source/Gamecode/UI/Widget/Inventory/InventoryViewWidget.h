// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryViewWidget.generated.h"

class UGridPanel;
class UInventorySlotWidget;
class UInventorySlot;

UCLASS()
class GAMECODE_API UInventoryViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeViewWidget(TArray<UInventorySlot*>& InventorySlots);

protected:

	UPROPERTY(meta = (BindWidget))
	UGridPanel* GridPanelItemSlots;

	UPROPERTY(EditDefaultsOnly, Category = "ItemContainer View Settings")
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "ItemContainer View Settings")
	int32 ColumnCount = 4;

	void AddItemSlotView(UInventorySlot& SlotToAdd);

};
