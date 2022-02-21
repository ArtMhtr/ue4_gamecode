// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCodeTypes.h"
#include "EquipmentViewWidget.generated.h"

class UVerticalBox;
class UEquipmentSlotWidget;
class UCharacterEquipmentComponent;
class AEquipableItem;

UCLASS()
class GAMECODE_API UEquipmentViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEquipmentWidget(UCharacterEquipmentComponent* CharacterEquipmentComponent);

protected:
	void AddEquipmentSlotView(AEquipableItem* LinkToWeapon, int32 SlotIndex);
	void UpdateSlot(int32 SlotIndex);

	bool EquipEquipmentToSlot(const TSubclassOf<AEquipableItem>& WeaponClass, int32 SenderIndex);
	void RemoveEquipmentFromSlot(int32 SlotIndex);

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VBWeaponSlot;

	UPROPERTY(EditDefaultsOnly, Category = "ItemContainer View Settings")
	TSubclassOf<UEquipmentSlotWidget> DefaultSlotViewClass;

	TWeakObjectPtr<UCharacterEquipmentComponent> LinkedEquipmentComponent;

private:
	bool IsEquipmentWidgetVisibleSlot(EEquipmentSlots EquipmentSlot) const;
};
